#ifndef WM_HPP_
#define WM_HPP_

#include "workspace.hpp"
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <cstring>
#include <vector>

class WindowManager {
public:
    /* Singleton since only a single WM is required at once. */
    static WindowManager* GetInstance();
    ~WindowManager();
    void Run();
private:
    static WindowManager* instance_;
    WindowManager(Display* dpy);
    
    /* XEvent handlers. */
    void OnCreateNotify();
    void OnDestroyNotify();
    void OnMapRequest();
    void OnKeyPress();
    void OnButtonPress();
    void OnButtonRelease();
    void OnMotionNotify();

    /* Workspace related */
    void GotoWorkspace(short n);

    /* Client related */
    void Decorate(Window w);
    
    Display* dpy_;
    XEvent event_;
    XWindowAttributes attr_;
    XButtonEvent start_;
    bool fullscreen_;

    /* Workspaces */
    std::vector<Workspace*> workspaces_;
    short current_;
};

#endif
