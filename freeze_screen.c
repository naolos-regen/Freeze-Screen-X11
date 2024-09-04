#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PASSWORD "secret"  // Define the password for demonstration purposes

Display *display;
Window overlay_window;
GC gc;
XFontStruct *font_info;
char input_buffer[128] = "";
int input_length = 0;

#define SCREEN_MIDDLE(screen_width, text_width) ((screen_width - text_width) / 2)

// Function to create the overlay window
void create_overlay_window() {
    int screen = DefaultScreen(display);
    Window root = RootWindow(display, screen);
    unsigned int screen_width = DisplayWidth(display, screen);
    unsigned int screen_height = DisplayHeight(display, screen);

    // Create the overlay window
    overlay_window = XCreateSimpleWindow(display, root,
                                         0, 0, screen_width,
                                         screen_height, 0,
                                         BlackPixel(display, screen),
                                         BlackPixel(display, screen));

    XClassHint *classHint = XAllocClassHint();
    classHint->res_name = "fullscreen";
    classHint->res_class = "Fullscreen";
    XSetClassHint(display, overlay_window, classHint);
    XFree(classHint);
    XSetWindowAttributes attr;
    attr.override_redirect = True;
    XChangeWindowAttributes(display, overlay_window, CWOverrideRedirect, &attr);

    XSelectInput(display, overlay_window, ExposureMask | KeyPressMask);
    XStoreName(display, overlay_window, "Password Required");
    XMapWindow(display, overlay_window);

    // Create a graphics context for drawing text
    gc = XCreateGC(display, overlay_window, 0, NULL);
    font_info = XLoadQueryFont(display, "fixed");
    XSetFont(display, gc, font_info->fid);

    XFlush(display);
}

// Function to handle input from the overlay window
void handle_overlay_input(XEvent *event) {
    int screen_width = DisplayWidth(display, DefaultScreen(display));
    int screen_height = DisplayHeight(display, DefaultScreen(display));

    if (event->type == KeyPress) {
        KeySym key;
        char buffer[1];
        XComposeStatus compose;
        XLookupString(&event->xkey, buffer, sizeof(buffer), &key, &compose);

        if (buffer[0] == '\r') {  // Enter key
            if (strcmp(input_buffer, PASSWORD) == 0) {
                printf("Password correct. Removing overlay...\n");
                XClearWindow(display, overlay_window);
                XDestroyWindow(display, overlay_window);
                XFreeGC(display, gc);
                XFreeFont(display, font_info);
                XUngrabPointer(display, CurrentTime);
                XUngrabKeyboard(display, CurrentTime);
                exit(0);
            } else {
                printf("Incorrect password.\n");
                XClearWindow(display, overlay_window);

                // Set the text color to red for the incorrect password message
                XSetForeground(display, gc, 0xFF0000);  // 0xFF0000 is the hex color code for red

                // Center the "Enter Password:" text
                int text_width = XTextWidth(font_info, "Enter Password:", strlen("Enter Password:"));
                int text_x = SCREEN_MIDDLE(screen_width, text_width);
                int text_y = screen_height / 2 - 20;  // Slightly above the center

                // Center the "Incorrect password. Try again." text
                int error_width = XTextWidth(font_info, "Incorrect password. Try again.", strlen("Incorrect password. Try again."));
                int input_x = SCREEN_MIDDLE(screen_width, error_width);
                int input_y = screen_height / 2 + 20;  // Slightly below the center

                XDrawString(display, overlay_window, gc, text_x, text_y, "Enter Password:", strlen("Enter Password:"));
                XDrawString(display, overlay_window, gc, input_x, input_y, "Incorrect password. Try again.", strlen("Incorrect password. Try again."));
                input_length = 0;  // Clear the input buffer
            }
        } else if (buffer[0] == 27) {  // Escape key
            XClearWindow(display, overlay_window);
            XDestroyWindow(display, overlay_window);
            XFreeGC(display, gc);
            XFreeFont(display, font_info);
            XUngrabPointer(display, CurrentTime);
            XUngrabKeyboard(display, CurrentTime);
            exit(1);
        } else if (input_length < sizeof(input_buffer) - 1) {
            input_buffer[input_length++] = buffer[0];
            input_buffer[input_length] = '\0';

            // Redraw the window to show the input
            XClearWindow(display, overlay_window);

            // Set the text color to red for user input
            XSetForeground(display, gc, 0xFF0000);  // 0xFF0000 is the hex color code for red

            // Center the "Enter Password:" text
            int text_width = XTextWidth(font_info, "Enter Password:", strlen("Enter Password:"));
            int text_x = SCREEN_MIDDLE(screen_width, text_width);
            int text_y = screen_height / 2 - 20;  // Slightly above the center

            // Center the user input text
            int input_width = XTextWidth(font_info, input_buffer, input_length);
            int input_x = SCREEN_MIDDLE(screen_width, input_width);
            int input_y = screen_height / 2 + 20;  // Slightly below the center

            XDrawString(display, overlay_window, gc, text_x, text_y, "Enter Password:", strlen("Enter Password:"));
            XDrawString(display, overlay_window, gc, input_x, input_y, input_buffer, input_length);
        }
    } else if (event->type == Expose) {
        // Handle window expose events
        XClearWindow(display, overlay_window);

        // Set the text color to red for window exposure
        XSetForeground(display, gc, 0xFF0000);  // 0xFF0000 is the hex color code for red

        // Center the "Enter Password:" text
        int text_width = XTextWidth(font_info, "Enter Password:", strlen("Enter Password:"));
        int text_x = SCREEN_MIDDLE(screen_width, text_width);
        int text_y = screen_height / 2 - 20;  // Slightly above the center

        // Center the user input text
        int input_width = XTextWidth(font_info, input_buffer, input_length);
        int input_x = SCREEN_MIDDLE(screen_width, input_width);
        int input_y = screen_height / 2 + 20;  // Slightly below the center

        XDrawString(display, overlay_window, gc, text_x, text_y, "Enter Password:", strlen("Enter Password:"));
        XDrawString(display, overlay_window, gc, input_x, input_y, input_buffer, input_length);
    }
}

int main() {
    display = XOpenDisplay(NULL);
    if (display == NULL) {
        fprintf(stderr, "Unable to open X display\n");
        exit(1);
    }

    // Create the overlay window
    create_overlay_window();

    // Grab the pointer and keyboard to prevent interaction with other windows
    XGrabPointer(display, overlay_window, True,
                 ButtonPressMask | ButtonReleaseMask | PointerMotionMask,
                 GrabModeAsync, GrabModeAsync, overlay_window, None, CurrentTime);
    XGrabKeyboard(display, overlay_window, True, GrabModeAsync, GrabModeAsync, CurrentTime);

    // Main event loop
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
