#include <X11/Xutil.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <sys/wait.h>

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
    char command[BUFFER_SIZE];
    snprintf(command, BUFFER_SIZE, "echo %s | pamtester login %s authenticate", password, username); // For login authentication

    int retval = system(command);
    if (retval == 0) {
        return 0;  // Authentication successful
    } else {
        return -1;  // Authentication failed
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
    const char *video_command = "vlc --play-and-exit ./youareanidiot.webm";
    execute_command(video_command);
}

void handle_overlay_input(XEvent *event) {
    if (event->type == KeyPress) {
        KeySym key;
        char buffer[BUFFER_SIZE];
        int bytes = XLookupString(&event->xkey, buffer, sizeof(buffer), &key, NULL);

        // Handle "Enter" key press
        if (key == XK_Return) {
            if (state == 0) {  // Username entry
                snprintf(username, BUFFER_SIZE, "%s", input_buffer);  // Safer than strncpy
                input_length = 0;  // Reset input buffer
                input_buffer[0] = '\0';
                state = 1;  // Switch to password entry
            } else if (state == 1) {  // Password entry
                snprintf(password, BUFFER_SIZE, "%s", input_buffer);  // Safer than strncpy

                if (authenticate_user(username, password) == 0) {
                    // Authentication successful
                    XClearWindow(display, overlay_window);
                    XDestroyWindow(display, overlay_window);
                    XFreeGC(display, gc);
                    XFreeFont(display, font_info);
                    XUngrabPointer(display, CurrentTime);
                    XUngrabKeyboard(display, CurrentTime);

                    // Securely erase sensitive data from memory
                    memset(password, 0, BUFFER_SIZE);

                    exit(0);
                } else {
                    retry_count++;
                    if (retry_count >= MAX_RETRIES) {
                        play_video();  // Handle too many retries
                        retry_count = 0;  // Reset retries
                    } else {
                        state = 0;  // Go back to username entry
                    }
                }
                // Securely erase password after checking
                memset(password, 0, BUFFER_SIZE);
            }
            input_length = 0;  // Clear input buffer after return
            input_buffer[0] = '\0';
        }

        // Handle backspace key press
        else if (key == XK_BackSpace && input_length > 0) {
            input_buffer[--input_length] = '\0';  // Remove the last character
            update_display();
        }

        // Handle regular character input
        else if (bytes > 0) {
            int space_remaining = BUFFER_SIZE - input_length - 1;
            if (space_remaining > 0) {
                // Limit to the space remaining
                int copy_size = (bytes > space_remaining) ? space_remaining : bytes;
                strncat(input_buffer, buffer, copy_size);
                input_length += copy_size;
                input_buffer[input_length] = '\0';  // Ensure null-termination
                update_display();
            }
        }
    } else if (event->type == Expose) {
        update_display();
    }
}

int main() {
    display = XOpenDisplay(NULL);
    if (display == NULL) {
        fprintf(stderr, "Unable to open X display\n");
        exit(EXIT_FAILURE);
    }

    create_overlay_window();

    XGrabPointer(display, overlay_window, True, ButtonPressMask | ButtonReleaseMask | PointerMotionMask, GrabModeAsync, GrabModeAsync, overlay_window, None, CurrentTime);
    XGrabKeyboard(display, overlay_window, True, GrabModeAsync, GrabModeAsync, CurrentTime);

    XEvent event;
    while (1) {
        XNextEvent(display, &event);
        if (event.xany.window == overlay_window) {
            handle_overlay_input(&event);
        }
    }

    // Cleanup and secure memory before exiting
    XDestroyWindow(display, overlay_window);
    XFreeGC(display, gc);
    XFreeFont(display, font_info);
    XCloseDisplay(display);

    // Securely erase sensitive data from memory
    memset(password, 0, BUFFER_SIZE);

    return 0;
}
