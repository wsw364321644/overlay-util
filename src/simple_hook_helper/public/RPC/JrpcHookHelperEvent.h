#pragma once

#include <RPC/rpc_processer.h>
#include <RPC/jrpc_interface.h>
#include "RpcHookHelperEventInterface.h"
#include "HOOK/keyboard_event.h"
#include "HOOK/mouse_event.h"
#include "HOOK/window_event.h"
#include "HOOK/hotkey_list.h"
#include "HOOK/input_event.h"
#include "std_ext.h"
#include <any>
#include <set>
#pragma warning(push)
#pragma warning(disable:4251)

struct HotKeyNode_t;

typedef std::set<HotKeyNode_t> HotKeyList_t;

typedef struct HotKeyNode_t {
    key_with_modifier_t HotKey;
    std::variant<HotKeyList_t, std::string> Child;
    bool operator<(const HotKeyNode_t& other) const
    {
        return HotKey.key_code<other.HotKey.key_code;
    }
}HotKeyNode_t;

namespace std
{
    template <>
    struct hash<HotKeyNode_t>
    {
        std::size_t operator()(const HotKeyNode_t& c) const
        {
            return c.HotKey.key_code;
        }
    };
}

   //std::any is std::string or HotKeyListNode_t


class HOOK_HELPER_EXPORT JRPCHookHelperEventAPI :public IGroupJRPC, public IRPCHookHelperEventAPI
{
public:
    DECLARE_RPC_OVERRIDE_FUNCTION(JRPCHookHelperEventAPI);

    //to overlay
    DECLARE_REQUEST_RPC_EVENT_ONE_PARAM(HotkeyListUpdate, HotKeyList_t&);
    DECLARE_REQUEST_RPC_EVENT_ONE_PARAM(InputStateUpdate, overlay_ime_event_t&);
    //to render
    DECLARE_REQUEST_RPC_EVENT_ONE_PARAM(ClientSizeUpdate, window_resize_event_t&);
    DECLARE_REQUEST_RPC_EVENT_TWO_PARAM(OverlayMouseWheelEvent, uint64_t, mouse_wheel_event_t&);
    DECLARE_REQUEST_RPC_EVENT_TWO_PARAM(OverlayMouseButtonEvent, uint64_t, mouse_button_event_t&);
    DECLARE_REQUEST_RPC_EVENT_TWO_PARAM(OverlayMouseMotionEvent, uint64_t, mouse_motion_event_t&);
    DECLARE_REQUEST_RPC_EVENT_TWO_PARAM(OverlayKeyboardEvent, uint64_t, keyboard_event_t&);
    DECLARE_REQUEST_RPC_EVENT_TWO_PARAM(OverlayCharEvent, uint64_t, overlay_char_event_t&);
    DECLARE_REQUEST_RPC_EVENT_TWO_PARAM(OverlayWindowEvent, uint64_t, window_event_t&);
};
#pragma warning(pop)
