#include <X11/Xutil.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>

#define BUFFER_SIZE 128
#define SCREEN_MIDDLE(screen_width, text_width) ((screen_width - text_width) / 2)
#define MAX_RETRIES 3

Display *display;
Window overlay_window;
GC gc;
XFontStruct *font_info;
char input_buffer[BUFFER_SIZE] = "";
char username[BUFFER_SIZE] = "";
char password[BUFFER_SIZE] = "";
int input_length = 0;
int state = 0; // 0 for username, 1 for password
int retry_count = 0; // Track the number of failed attempts

void execute_command(const char *command) {
    int retval = system(command);
    if (retval == -1) {
        perror("system");
    }
}

int authenticate_user(const char *username, const char *password) {
    char command[BUFFER_SIZE * 2];
    snprintf(command, sizeof(command), "./auth_script.sh %s %s", username, password);

    int retval = system(command);

    if (retval == -1) {
        perror("system");
        return -1; // Return failure code
    } else if (WIFEXITED(retval) && WEXITSTATUS(retval) == 0) {
        // Command executed successfully
        return 0; // Authentication successful
    } else {
        // Command failed
        return -1; // Authentication failed
    }
}

void create_overlay_window() {
    int screen = DefaultScreen(display);
    Window root = RootWindow(display, screen);
    unsigned int screen_width = DisplayWidth(display, screen);
    unsigned int screen_height = DisplayHeight(display, screen);

    overlay_window = XCreateSimpleWindow(display, root,
                                         0, 0, screen_width,
                                         screen_height, 0,
                                         BlackPixel(display, screen),
                                         BlackPixel(display, screen));

    XClassHint *classHint = XAllocClassHint();
    if (!classHint) {
        perror("XAllocClassHint");
        exit(EXIT_FAILURE);
    }
    classHint->res_name = "fullscreen";
    classHint->res_class = "Fullscreen";
    XSetClassHint(display, overlay_window, classHint);
    XFree(classHint);

    XSetWindowAttributes attr;
    attr.override_redirect = True;
    XChangeWindowAttributes(display, overlay_window, CWOverrideRedirect, &attr);

    XSelectInput(display, overlay_window, ExposureMask | KeyPressMask);
    XStoreName(display, overlay_window, "Authentication Required");
    XMapWindow(display, overlay_window);

    gc = XCreateGC(display, overlay_window, 0, NULL);
    font_info = XLoadQueryFont(display, "fixed");
    if (!font_info) {
        perror("XLoadQueryFont");
        exit(EXIT_FAILURE);
    }
    XSetFont(display, gc, font_info->fid);

    XFlush(display);
}

void draw_current_state() {
    XSetForeground(display, gc, 0xFF0000);  // Set text color to red

    int screen_width = DisplayWidth(display, DefaultScreen(display));
    int screen_height = DisplayHeight(display, DefaultScreen(display));

    if (state == 0) { // Username entry
        int text_width = XTextWidth(font_info, "Enter Username:", strlen("Enter Username:"));
        int text_x = SCREEN_MIDDLE(screen_width, text_width);
        int text_y = screen_height / 2 - 20;

        int input_width = XTextWidth(font_info, input_buffer, input_length);
        int input_x = SCREEN_MIDDLE(screen_width, input_width);
        int input_y = screen_height / 2 + 20;

        XDrawString(display, overlay_window, gc, text_x, text_y, "Enter Username:", strlen("Enter Username:"));
        XDrawString(display, overlay_window, gc, input_x, input_y, input_buffer, input_length);
    } else if (state == 1) { // Password entry
        int text_width = XTextWidth(font_info, "Enter Password:", strlen("Enter Password:"));
        int text_x = SCREEN_MIDDLE(screen_width, text_width);
        int text_y = screen_height / 2 - 20;

        // Display password as asterisks
        char masked_password[BUFFER_SIZE];
        memset(masked_password, '*', input_length);
        masked_password[input_length] = '\0';

        int input_width = XTextWidth(font_info, masked_password, input_length);
        int input_x = SCREEN_MIDDLE(screen_width, input_width);
        int input_y = screen_height / 2 + 20;

        XDrawString(display, overlay_window, gc, text_x, text_y, "Enter Password:", strlen("Enter Password:"));
        XDrawString(display, overlay_window, gc, input_x, input_y, masked_password, input_length);
    }
}

void update_display() {
    XClearWindow(display, overlay_window);
    draw_current_state();
}

void play_video() {
    // Replace 'path_to_your_video.mp4' with the actual path to your video file
    const char *video_command = "vlc --play-and-exit /home/naolos/Documents/Programming/freezing/youareanidiot.webm";
    execute_command(video_command);
}

void handle_overlay_input(XEvent *event) {
    if (event->type == KeyPress) {
        KeySym key;
        char buffer[1];
        XComposeStatus compose;
        XLookupString(&event->xkey, buffer, sizeof(buffer), &key, &compose);

        if (key == XK_Return) {  // Enter key
            if (state == 0) { // Username entry
                strncpy(username, input_buffer, BUFFER_SIZE - 1);
                username[BUFFER_SIZE - 1] = '\0';
                input_length = 0;
                input_buffer[0] = '\0';
                state = 1; // Switch to password entry
                printf("Username entered. Please enter password:\n");
            } else if (state == 1) { // Password entry
                if (authenticate_user(username, input_buffer) == 0) {
                    printf("Authentication successful.\n");
                    // Clear and destroy the overlay window
                    XClearWindow(display, overlay_window);
                    XDestroyWindow(display, overlay_window);
                    XFreeGC(display, gc);
                    XFreeFont(display, font_info);
                    XUngrabPointer(display, CurrentTime);
                    XUngrabKeyboard(display, CurrentTime);
                    exit(0);
                } else {
                    retry_count++;
                    if (retry_count >= MAX_RETRIES) {
                        printf("Maximum retries reached. Playing video.\n");
                        play_video();
                        retry_count = 0; // Reset retry count after playing video
                    } else {
                        printf("Incorrect password. Try again.\n");
                        XClearWindow(display, overlay_window);

                        XSetForeground(display, gc, 0xFF0000);  // Set text color to red

                        int screen_width = DisplayWidth(display, DefaultScreen(display));
                        int screen_height = DisplayHeight(display, DefaultScreen(display));

                        int text_width = XTextWidth(font_info, "Enter Username:", strlen("Enter Username:"));
                        int text_x = SCREEN_MIDDLE(screen_width, text_width);
                        int text_y = screen_height / 2 - 40;

                        int error_width = XTextWidth(font_info, "Incorrect password. Try again.", strlen("Incorrect password. Try again."));
                        int input_x = SCREEN_MIDDLE(screen_width, error_width);
                        int input_y = screen_height / 2 + 20;

                        XDrawString(display, overlay_window, gc, text_x, text_y, "Enter Username:", strlen("Enter Username:"));
                        XDrawString(display, overlay_window, gc, input_x, input_y, "Incorrect password. Try again.", strlen("Incorrect password. Try again."));
                        input_length = 0;  // Clear the input buffer
                        state = 0; // Switch back to username entry
                    }
                }
            }
        } else if (key == XK_Escape) {  // Escape key
            XClearWindow(display, overlay_window);
            XDestroyWindow(display, overlay_window);
            XFreeGC(display, gc);
            XFreeFont(display, font_info);
            XUngrabPointer(display, CurrentTime);
            XUngrabKeyboard(display, CurrentTime);
            exit(1);
        } else if (key == XK_BackSpace) {  // Backspace key
            if (input_length > 0) {
                input_buffer[--input_length] = '\0';
                XClearWindow(display, overlay_window);
                update_display();  // Function to update the display
            }
        } else if (key >= XK_space && key <= XK_asciitilde && input_length < BUFFER_SIZE - 1) {
            // Ensure printable characters are within range
            if (key >= XK_space && key <= XK_asciitilde) {
                buffer[0] = (char)key;
                input_buffer[input_length++] = buffer[0];
                input_buffer[input_length] = '\0';
                XClearWindow(display, overlay_window);
                update_display();  // Function to update the display
            }
        }
    } else if (event->type == Expose) {
        XClearWindow(display, overlay_window);
        update_display();  // Function to update the display
    }
}

int main() {
    display = XOpenDisplay(NULL);
    if (display == NULL) {
        fprintf(stderr, "Unable to open X display\n");
        exit(EXIT_FAILURE);
    }

    create_overlay_window();

    XGrabPointer(display, overlay_window, True,
                 ButtonPressMask | ButtonReleaseMask | PointerMotionMask,
                 GrabModeAsync, GrabModeAsync, overlay_window, None, CurrentTime);
    XGrabKeyboard(display, overlay_window, True, GrabModeAsync, GrabModeAsync, CurrentTime);

    XEvent event;
    while (1) {
        XNextEvent(display, &event);
        if (event.xany.window == overlay_window) {
            handle_overlay_input(&event);
        }
    }

    // Clean up (this will never be reached in this example)
    XDestroyWindow(display, overlay_window);
    XFreeGC(display, gc);
    XFreeFont(display, font_info);
    XCloseDisplay(display);

    return 0;
}
