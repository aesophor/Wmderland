#include "util.h"
#include <sstream>

using std::pair;
using std::size_t;
using std::string;
using std::vector;

Area::Area() {
    Area(0, 0, 0, 0);
}

Area::Area(int x, int y, int width, int height) : x(x), y(y), width(width), height(height) {}


bool Area::operator==(const Area& other) {
    return (x == other.x) && (y == other.y) && (width == other.width) && (height == other.height);
}

bool Area::operator!=(const Area& other) {
    return (x != other.x) || (y != other.y) || (width != other.width) || (height != other.height);
}



namespace wm_utils {

namespace {
    Display* dpy;
    Properties* prop;
    Window root_window;
} // namespace

void Init(Display* d, Properties* p, Window root_win) {
    dpy = d;
    prop = p;
    root_window = root_win;
}
 
pair<int, int> GetDisplayResolution() {
    XWindowAttributes root_window_attr = GetXWindowAttributes(root_window);
    return pair<short, short>(root_window_attr.width, root_window_attr.height);
}

// Get the XWindowAttributes of window w.
XWindowAttributes GetXWindowAttributes(Window w) {
    XWindowAttributes ret;
    XGetWindowAttributes(dpy, w, &ret);
    return ret;
}

// Get the XSizeHints of window w.
XSizeHints GetWmNormalHints(Window w) {
    XSizeHints hints;
    long msize;
    XGetWMNormalHints(dpy, w, &hints, &msize);
    return hints;
}

// Get the XClassHint (which contains res_class and res_name) of window w.
pair<string, string> GetXClassHint(Window w) {
    XClassHint hint;

    if (XGetClassHint(dpy, w, &hint)) {
        static const string undefined = "undefined";
        string res_class = (hint.res_class) ? hint.res_class : undefined;
        string res_name = (hint.res_name) ? hint.res_name : undefined;

        if (hint.res_class) {
            XFree(hint.res_class);
        }
        if (hint.res_name) {
            XFree(hint.res_name);
        }
        return std::make_pair(res_class, res_name);
    } 
    
    return std::make_pair("", "");
}

// Get the utf8string in _NET_WM_NAME property.
string GetNetWmName(Window w) {
    XTextProperty name;
    if (!XGetTextProperty(dpy, w, &name, prop->net[atom::NET_WM_NAME]) || !name.nitems) {
        return string();
    }
    string ret = string((char*) name.value);
    XFree(name.value);
    return ret;
}

// Get the WM_NAME (i.e., the window title) of window w.
string GetWmName(Window w) {
    Atom prop = XInternAtom(dpy, "WM_NAME", False), type;
    int form;
    unsigned long remain, len;
    unsigned char* list;

    if (XGetWindowProperty(dpy, w, prop, 0, 1024, False, 
                AnyPropertyType, &type, &form, &len, &remain, &list) == Success) {
        return string((char*) list);
    }
    return nullptr;
}

// Set WM_STATE according to the following page to fix WINE application close hang issue:
// http://www.x.org/releases/X11R7.7/doc/xorg-docs/icccm/icccm.html#WM_STATE_Property
void SetWindowWmState(Window w, unsigned long state) {
    unsigned long wm_state[] = { state, None };
    XChangeProperty(dpy, w,  prop->wm[atom::WM_STATE], prop->wm[atom::WM_STATE], 32,
            PropModeReplace, (unsigned char*) wm_state, 2);
}

// Set root window's _NET_ACTIVE_WINDOW property
void SetNetActiveWindow(Window w) {
    XChangeProperty(dpy, root_window, prop->net[atom::NET_ACTIVE_WINDOW], XA_WINDOW, 32,
            PropModeReplace, (unsigned char*) &w, 1);
}

// Clear root window's _NET_ACTIVE_WINDOW property
void ClearNetActiveWindow() {
    XDeleteProperty(dpy, root_window, prop->net[atom::NET_ACTIVE_WINDOW]);
}

// Get the atoms contained in the property of window w. The number of atoms retrieved
// will be stored in *atom_len. XFree() should be called manually on the returned Atom ptr.
Atom* GetWindowProperty(Window w, Atom property, unsigned long* atom_len) {
    Atom da;
    unsigned char *prop_ret = nullptr;
    int di;
    unsigned long remain;
        
    if (XGetWindowProperty(dpy, w, property, 0, sizeof(Atom), False,
                XA_ATOM, &da, &di, atom_len, &remain, &prop_ret) == Success && prop_ret) {
        return (Atom*) prop_ret;
    }
    return nullptr;
}

// Check if the property of window w contains the target atom.
bool WindowPropertyHasAtom(Window w, Atom property, Atom target_atom) {
    unsigned long atom_len = 0;
    Atom* atoms = GetWindowProperty(w, property, &atom_len);

    for (int i = 0; atoms && i < (int) atom_len; i++) {
        if (atoms[i] && atoms[i] == target_atom) {
            XFree(atoms);
            return true;
        }
    }
    XFree(atoms);
    return false;
}


bool HasNetWmStateFullscreen(Window w) {
    return WindowPropertyHasAtom(w, prop->net[atom::NET_WM_STATE], prop->net[atom::NET_WM_STATE_FULLSCREEN]);
}

bool IsWindowOfType(Window w, Atom type_atom) {
    return WindowPropertyHasAtom(w, prop->net[atom::NET_WM_WINDOW_TYPE], type_atom);
}

bool IsDock(Window w) {
    return IsWindowOfType(w, prop->net[atom::NET_WM_WINDOW_TYPE_DOCK]);
}

bool IsDialog(Window w) {
    return IsWindowOfType(w, prop->net[atom::NET_WM_WINDOW_TYPE_DIALOG]);
}

bool IsSplash(Window w) {
    return IsWindowOfType(w, prop->net[atom::NET_WM_WINDOW_TYPE_SPLASH]);
}

bool IsUtility(Window w) {
    return IsWindowOfType(w, prop->net[atom::NET_WM_WINDOW_TYPE_UTILITY]);
}

bool IsNotification(Window w) {
    return IsWindowOfType(w, prop->net[atom::NET_WM_WINDOW_TYPE_NOTIFICATION]);
}



string KeysymToStr(unsigned int keycode) {
    return string(XKeysymToString(XkbKeycodeToKeysym(dpy, keycode, 0, false))); 
}

unsigned int StrToKeycode(const string& key_name) {
    return XKeysymToKeycode(dpy, XStringToKeysym(key_name.c_str()));
}
    
string KeymaskToStr(int modifier) {
    switch (modifier) {
        case Mod4Mask: // Cmd
        case Mod4Mask | LockMask: // Cmd + Caps
            return "Mod4";
        case Mod4Mask | ShiftMask: // Cmd + Shift
        case Mod4Mask | ShiftMask | LockMask: // Cmd + Shift + Caps
            return "Mod4+Shift";

        case Mod1Mask: // Alt
        case Mod1Mask | LockMask: // Alt + Caps
            return "Mod1";
        case Mod1Mask | ShiftMask: // Alt + Shift
        case Mod1Mask | ShiftMask | LockMask: // Alt + Shift + Caps
            return "Mod1+Shift";

        case Mod2Mask:
        case Mod2Mask | LockMask:
            return "Mod2";
        case Mod2Mask | ShiftMask:
        case Mod2Mask | ShiftMask | LockMask:
            return "Mod2+Shift";

        case Mod3Mask:
        case Mod3Mask | LockMask:
            return "Mod3";
        case Mod3Mask | ShiftMask:
        case Mod3Mask | ShiftMask | LockMask:
            return "Mod3+Shift";

        case Mod5Mask:
        case Mod5Mask | LockMask:
            return "Mod5";
        case Mod5Mask | ShiftMask:
        case Mod5Mask | ShiftMask | LockMask:
            return "Mod5+Shift";

        default:
            return "";
    }
}

int StrToKeymask(const string& modifier, bool shift) {
    static int mod_masks[6] = { 0, Mod1Mask, Mod2Mask, Mod3Mask, Mod4Mask, Mod5Mask };
    int modifier_id = modifier.at(3) - '0'; // "Mod4": 0123

    if (modifier_id >= 1 && modifier_id <= 5) {
        int mod_mask = mod_masks[modifier_id];
        return (shift) ? mod_mask | ShiftMask : mod_mask;
    } else {
        return None;
    }
}

} // namespace wm_utils



namespace string_utils {

vector<string> Split(const string& s, const char delimiter) {
    std::stringstream ss(s);
    string t;
    vector<string> tokens;

    while (std::getline(ss, t, delimiter)) {
        if (t.length() > 0) {
            tokens.push_back(t);
        }
    }
    return tokens;
}

vector<string> Split(const string& s, const char delimiter, int count) {
    vector<string> tokens;
    string::size_type head = 0;
    string::size_type tail = s.find(delimiter, head);

    for (short i = 0; i < count; i++) {
        string t = s.substr(head, tail - head);
        if (t.length() > 0) {
            tokens.push_back(t);
        }
        head = tail + 1;
        tail = s.find(delimiter, tail + 1);
    }

    if (head != 0) {
        string t = s.substr(head, string::npos);
        if (t.length() > 0) {
            tokens.push_back(t);
        }
    }
    return tokens;
}

bool StartsWith(const string& s, const string& keyword) {
    return s.find(keyword) == 0;
}

bool Contains(const string& s, const string& keyword) {
    return s.find(keyword) != string::npos;
}

void Replace(string& s, const string& keyword, const string& newword) {
    string::size_type pos = s.find(keyword);

    while(pos != std::string::npos) {
        s.replace(pos, keyword.size(), newword);
        pos = s.find(keyword, pos + newword.size());
    }
}

void Strip(string& s) {
    static const char* whitespace_chars = " \n\r\t";
    s.erase(0, s.find_first_not_of(whitespace_chars));
    s.erase(s.find_last_not_of(whitespace_chars) + 1);
}

} // namespace string_utils



namespace sys_utils {
    
string ToAbsPath(const string& path) {
    string abs_path = path;

    if (path.at(0) == '~') {
        abs_path = string(getenv("HOME")) + path.substr(1, string::npos);
    }

    return abs_path;
}

} // namespace sys_utils