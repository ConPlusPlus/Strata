/* Copyright © 2026 Connor Rutberg */
/* crossplatform.h - one-file platform layer for Strata programs and C/C++ code.
 *
 * Single-header library: the declarations are always visible; the implementation is
 * compiled only where it's switched on.
 *
 * USING IT
 *   Strata:  import <crossplatform.h>
 *            The implementation switches on automatically (stratac defines
 *            STRATA_PROGRAM, and a Strata program is always one C file).
 *   C/C++:   #include "crossplatform.h" anywhere, and in exactly ONE .c/.cpp file:
 *                #define STRATA_CROSSPLATFORM
 *                #include "crossplatform.h"
 *
 * SECTIONS (each can be left out, e.g. to avoid linking its OS libraries)
 *   Window   opt out: #define STRATA_CROSSPLATFORM_NO_WINDOW
 *
 * LINKING (per section)
 *   Window   Windows: user32  (Strata: link "user32"; MSVC links it automatically)
 *            Linux:   X11     (Strata: link "X11")
 *            macOS:   -framework Cocoa -lobjc
 *
 * ADDING A SECTION
 *   1. declarations in the DECLARATIONS part, inside #ifndef STRATA_CROSSPLATFORM_NO_<NAME>
 *   2. its OS headers in the PLATFORM part, under the same guard
 *   3. the implementation in the IMPLEMENTATION part, one #if branch per platform
 *   4. list it under SECTIONS and LINKING above
 */

/* ============================== DECLARATIONS ============================== */

#ifndef STRATA_CROSSPLATFORM_H
#define STRATA_CROSSPLATFORM_H

#ifdef __cplusplus
extern "C" {
#endif

/* ---- Window ---------------------------------------------------------------- */
#ifndef STRATA_CROSSPLATFORM_NO_WINDOW

    /* Creates and shows a window whose client area is width x height. title is UTF-8. */
    void CreatePlatformWindow(int width, int height, const char* title);

    /* Processes pending OS events. Returns 1 while the window is open, 0 once it has been closed. */
    int PlatformPollEvents(void);

    /* Changes the window title (UTF-8). */
    void PlatformSetWindowTitle(const char* title);

    /* Resizes the window's client area. */
    void PlatformSetWindowSize(int width, int height);

    /* Current client-area size, including after the user resizes the window. */
    int PlatformGetWindowWidth(void);
    int PlatformGetWindowHeight(void);

    /* Closes the window. PlatformPollEvents returns 0 afterwards. */
    void PlatformCloseWindow(void);

#endif /* STRATA_CROSSPLATFORM_NO_WINDOW */

#ifdef __cplusplus
}
#endif

#endif /* STRATA_CROSSPLATFORM_H */

/* ============================= IMPLEMENTATION ============================= */

#if defined(STRATA_CROSSPLATFORM) || defined(STRATA_PROGRAM)
#ifndef STRATA_CROSSPLATFORM_IMPLEMENTED
#define STRATA_CROSSPLATFORM_IMPLEMENTED

/* ---- PLATFORM: detection + OS headers (outside extern "C") ----------------- */

#if defined(_WIN32)
    #define STRATA__WINDOWS 1
    #ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
    #endif
    #include <windows.h>
#elif defined(__APPLE__)
    #define STRATA__MACOS 1
    #ifndef STRATA_CROSSPLATFORM_NO_WINDOW
    #include <objc/runtime.h>
    #include <objc/message.h>
    #include <CoreGraphics/CoreGraphics.h>
    #endif
#elif defined(__linux__)
    #define STRATA__LINUX 1
    #ifndef STRATA_CROSSPLATFORM_NO_WINDOW
    #include <X11/Xlib.h>
    #include <X11/Xutil.h>
    #endif
#else
    #error "crossplatform.h: unsupported platform (expected Windows, macOS or Linux)"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* ---- Window ---------------------------------------------------------------- */
#ifndef STRATA_CROSSPLATFORM_NO_WINDOW

    static int strata__width = 0;
    static int strata__height = 0;

    int PlatformGetWindowWidth(void) { return strata__width; }
    int PlatformGetWindowHeight(void) { return strata__height; }

#if defined(STRATA__WINDOWS)
#ifdef _MSC_VER
#pragma comment(lib, "user32.lib")
#endif

#define STRATA__STYLE WS_OVERLAPPEDWINDOW

    static HWND strata__hwnd = NULL;

    static LRESULT CALLBACK strata__WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
        switch (msg) {
        case WM_SIZE:
            if (wParam != SIZE_MINIMIZED) {
                strata__width = LOWORD(lParam);
                strata__height = HIWORD(lParam);
            }
            return 0;
        case WM_CLOSE:
            DestroyWindow(hwnd);
            return 0;
        case WM_DESTROY:
            strata__hwnd = NULL;
            return 0;
        }
        return DefWindowProcW(hwnd, msg, wParam, lParam);
    }

    static void strata__widen(const char* utf8, wchar_t* out, int outLen) {
        if (!utf8 || !MultiByteToWideChar(CP_UTF8, 0, utf8, -1, out, outLen))
            out[0] = L'\0';
    }

    void CreatePlatformWindow(int width, int height, const char* title) {
        static const wchar_t* className = L"StrataWindowClass";
        static int registered = 0;
        HINSTANCE instance = GetModuleHandleW(NULL);
        RECT rect = { 0, 0, width, height };
        wchar_t wtitle[256];

        if (strata__hwnd) return;

        if (!registered) {
            WNDCLASSEXW wc;
            ZeroMemory(&wc, sizeof(wc));
            wc.cbSize = sizeof(wc);
            wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
            wc.lpfnWndProc = strata__WndProc;
            wc.hInstance = instance;
            wc.hCursor = LoadCursor(NULL, IDC_ARROW);
            wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
            wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
            wc.lpszClassName = className;
            if (!RegisterClassExW(&wc)) return;
            registered = 1;
        }

        /* Grow the outer rect so the client area is exactly width x height. */
        AdjustWindowRect(&rect, STRATA__STYLE, FALSE);
        strata__widen(title, wtitle, 256);

        strata__hwnd = CreateWindowExW(0, className, wtitle, STRATA__STYLE,
            CW_USEDEFAULT, CW_USEDEFAULT,
            rect.right - rect.left, rect.bottom - rect.top,
            NULL, NULL, instance, NULL);
        if (!strata__hwnd) return;

        strata__width = width;
        strata__height = height;
        ShowWindow(strata__hwnd, SW_SHOW);
        UpdateWindow(strata__hwnd);
    }

    int PlatformPollEvents(void) {
        MSG msg;
        while (PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }
        return strata__hwnd != NULL;
    }

    void PlatformSetWindowTitle(const char* title) {
        wchar_t wtitle[256];
        if (!strata__hwnd) return;
        strata__widen(title, wtitle, 256);
        SetWindowTextW(strata__hwnd, wtitle);
    }

    void PlatformSetWindowSize(int width, int height) {
        RECT rect = { 0, 0, width, height };
        if (!strata__hwnd) return;
        AdjustWindowRect(&rect, STRATA__STYLE, FALSE);
        SetWindowPos(strata__hwnd, NULL, 0, 0, rect.right - rect.left, rect.bottom - rect.top,
            SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
    }

    void PlatformCloseWindow(void) {
        if (strata__hwnd) DestroyWindow(strata__hwnd);
    }

#undef STRATA__STYLE

#elif defined(STRATA__MACOS)
    /* Plain C Cocoa via the Objective-C runtime, so this header works from .c/.cpp files. */

#define strata__cls(name) ((id)objc_getClass(name))
#define strata__sel(name) sel_registerName(name)

    static id  strata__app = NULL;
    static id  strata__window = NULL;
    static int strata__open = 0;

    static id strata__pool(void) {
        return ((id(*)(id, SEL))objc_msgSend)(strata__cls("NSAutoreleasePool"), strata__sel("new"));
    }

    static void strata__drain(id pool) {
        ((void (*)(id, SEL))objc_msgSend)(pool, strata__sel("drain"));
    }

    static id strata__nsstring(const char* utf8) {
        return ((id(*)(id, SEL, const char*))objc_msgSend)(
            strata__cls("NSString"), strata__sel("stringWithUTF8String:"), utf8 ? utf8 : "");
    }

    static void strata__updateSize(void) {
        id view = ((id(*)(id, SEL))objc_msgSend)(strata__window, strata__sel("contentView"));
        CGRect frame;
#if defined(__x86_64__)
        frame = ((CGRect(*)(id, SEL))objc_msgSend_stret)(view, strata__sel("frame"));
#else
        frame = ((CGRect(*)(id, SEL))objc_msgSend)(view, strata__sel("frame"));
#endif
        strata__width = (int)frame.size.width;
        strata__height = (int)frame.size.height;
    }

    static void strata__windowWillClose(id self, SEL cmd, id notification) {
        (void)self; (void)cmd; (void)notification;
        strata__open = 0;
    }

    static void strata__windowDidResize(id self, SEL cmd, id notification) {
        (void)self; (void)cmd; (void)notification;
        strata__updateSize();
    }

    void CreatePlatformWindow(int width, int height, const char* title) {
        id pool, delegate;
        Class delegateClass;
        CGRect frame;
        /* NSWindowStyleMaskTitled | Closable | Miniaturizable | Resizable */
        unsigned long styleMask = 1 | 2 | 4 | 8;

        if (strata__open) return;

        pool = strata__pool();

        strata__app = ((id(*)(id, SEL))objc_msgSend)(strata__cls("NSApplication"), strata__sel("sharedApplication"));
        /* NSApplicationActivationPolicyRegular: dock icon + menu bar even without an app bundle. */
        ((void (*)(id, SEL, long))objc_msgSend)(strata__app, strata__sel("setActivationPolicy:"), 0);
        ((void (*)(id, SEL))objc_msgSend)(strata__app, strata__sel("finishLaunching"));

        frame = CGRectMake(0, 0, width, height);
        strata__window = ((id(*)(id, SEL))objc_msgSend)(strata__cls("NSWindow"), strata__sel("alloc"));
        strata__window = ((id(*)(id, SEL, CGRect, unsigned long, unsigned long, BOOL))objc_msgSend)(
            strata__window, strata__sel("initWithContentRect:styleMask:backing:defer:"),
            frame, styleMask, 2 /* NSBackingStoreBuffered */, NO);
        if (!strata__window) {
            strata__drain(pool);
            return;
        }
        ((void (*)(id, SEL, BOOL))objc_msgSend)(strata__window, strata__sel("setReleasedWhenClosed:"), NO);

        /* Delegate class that tracks close and resize. */
        delegateClass = objc_getClass("StrataWindowDelegate");
        if (!delegateClass) {
            delegateClass = objc_allocateClassPair(objc_getClass("NSObject"), "StrataWindowDelegate", 0);
            class_addMethod(delegateClass, strata__sel("windowWillClose:"), (IMP)strata__windowWillClose, "v@:@");
            class_addMethod(delegateClass, strata__sel("windowDidResize:"), (IMP)strata__windowDidResize, "v@:@");
            objc_registerClassPair(delegateClass);
        }
        delegate = ((id(*)(id, SEL))objc_msgSend)((id)delegateClass, strata__sel("new"));
        ((void (*)(id, SEL, id))objc_msgSend)(strata__window, strata__sel("setDelegate:"), delegate);

        ((void (*)(id, SEL, id))objc_msgSend)(strata__window, strata__sel("setTitle:"), strata__nsstring(title));
        ((void (*)(id, SEL))objc_msgSend)(strata__window, strata__sel("center"));
        ((void (*)(id, SEL, id))objc_msgSend)(strata__window, strata__sel("makeKeyAndOrderFront:"), (id)NULL);
        ((void (*)(id, SEL, BOOL))objc_msgSend)(strata__app, strata__sel("activateIgnoringOtherApps:"), YES);

        strata__width = width;
        strata__height = height;
        strata__open = 1;
        strata__drain(pool);
    }

    int PlatformPollEvents(void) {
        id pool, event, distantPast, mode;

        if (!strata__app) return 0;

        pool = strata__pool();
        distantPast = ((id(*)(id, SEL))objc_msgSend)(strata__cls("NSDate"), strata__sel("distantPast"));
        mode = strata__nsstring("kCFRunLoopDefaultMode"); /* NSDefaultRunLoopMode */

        for (;;) {
            event = ((id(*)(id, SEL, unsigned long long, id, id, BOOL))objc_msgSend)(
                strata__app, strata__sel("nextEventMatchingMask:untilDate:inMode:dequeue:"),
                ~0ULL /* NSEventMaskAny */, distantPast, mode, YES);
            if (!event) break;
            ((void (*)(id, SEL, id))objc_msgSend)(strata__app, strata__sel("sendEvent:"), event);
        }
        ((void (*)(id, SEL))objc_msgSend)(strata__app, strata__sel("updateWindows"));

        strata__drain(pool);
        return strata__open;
    }

    void PlatformSetWindowTitle(const char* title) {
        id pool;
        if (!strata__open) return;
        pool = strata__pool();
        ((void (*)(id, SEL, id))objc_msgSend)(strata__window, strata__sel("setTitle:"), strata__nsstring(title));
        strata__drain(pool);
    }

    void PlatformSetWindowSize(int width, int height) {
        if (!strata__open) return;
        ((void (*)(id, SEL, CGSize))objc_msgSend)(strata__window, strata__sel("setContentSize:"),
            CGSizeMake(width, height));
        strata__updateSize();
    }

    void PlatformCloseWindow(void) {
        if (strata__open)
            ((void (*)(id, SEL))objc_msgSend)(strata__window, strata__sel("close"));
    }

#undef strata__cls
#undef strata__sel

#elif defined(STRATA__LINUX)

    static Display* strata__display = NULL;
    static Window   strata__window = 0;
    static Atom     strata__wmDeleteWindow;
    static int      strata__open = 0;

    void CreatePlatformWindow(int width, int height, const char* title) {
        int screen;

        if (strata__open) return;

        strata__display = XOpenDisplay(NULL);
        if (!strata__display) return;

        screen = DefaultScreen(strata__display);
        strata__window = XCreateSimpleWindow(strata__display, RootWindow(strata__display, screen),
            0, 0, (unsigned int)width, (unsigned int)height, 0,
            BlackPixel(strata__display, screen),
            BlackPixel(strata__display, screen));

        XStoreName(strata__display, strata__window, title ? title : "");
        XSelectInput(strata__display, strata__window,
            ExposureMask | KeyPressMask | KeyReleaseMask |
            ButtonPressMask | ButtonReleaseMask | PointerMotionMask |
            StructureNotifyMask | FocusChangeMask);

        /* Ask the window manager to send us a message instead of killing the connection on close. */
        strata__wmDeleteWindow = XInternAtom(strata__display, "WM_DELETE_WINDOW", False);
        XSetWMProtocols(strata__display, strata__window, &strata__wmDeleteWindow, 1);

        XMapWindow(strata__display, strata__window);
        XFlush(strata__display);
        strata__width = width;
        strata__height = height;
        strata__open = 1;
    }

    void PlatformCloseWindow(void) {
        if (!strata__open) return;
        XDestroyWindow(strata__display, strata__window);
        XCloseDisplay(strata__display);
        strata__display = NULL;
        strata__window = 0;
        strata__open = 0;
    }

    int PlatformPollEvents(void) {
        XEvent event;

        if (!strata__open) return 0;

        while (XPending(strata__display)) {
            XNextEvent(strata__display, &event);
            if (event.type == ConfigureNotify) {
                strata__width = event.xconfigure.width;
                strata__height = event.xconfigure.height;
            }
            else if (event.type == ClientMessage &&
                (Atom)event.xclient.data.l[0] == strata__wmDeleteWindow) {
                PlatformCloseWindow();
                return 0;
            }
        }
        return 1;
    }

    void PlatformSetWindowTitle(const char* title) {
        if (!strata__open) return;
        XStoreName(strata__display, strata__window, title ? title : "");
        XFlush(strata__display);
    }

    void PlatformSetWindowSize(int width, int height) {
        if (!strata__open) return;
        XResizeWindow(strata__display, strata__window, (unsigned int)width, (unsigned int)height);
        XFlush(strata__display);
        strata__width = width;
        strata__height = height;
    }

#endif /* platform */
#endif /* STRATA_CROSSPLATFORM_NO_WINDOW */

#ifdef __cplusplus
}
#endif

#endif /* STRATA_CROSSPLATFORM_IMPLEMENTED */
#endif /* STRATA_CROSSPLATFORM || STRATA_PROGRAM */
