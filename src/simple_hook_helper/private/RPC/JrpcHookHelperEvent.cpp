#include "RPC/JrpcHookHelperEvent.h"
#include "RPC/JrpcHookHelperInternal.h"

#include <crypto_lib_base64.h>
#include <RPC/message_common.h>
#include <jrpc_parser.h>
#include <simdjson.h>
#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

#include <typeindex>

constexpr int MOUSE_WHEEL_EVENT_BASE64_LEN = (sizeof(mouse_wheel_event_t) + 2) / 3 * 4+1;
constexpr int MOUSE_BOTTON_EVENT_BASE64_LEN = (sizeof(mouse_button_event_t) + 2) / 3 * 4+1;
constexpr int MOUSE_MOTION_EVENT_BASE64_LEN = (sizeof(mouse_motion_event_t) + 2) / 3 * 4+1;
constexpr int KEYBOARD_EVENT_BASE64_LEN = (sizeof(keyboard_event_t) + 2) / 3 * 4+1;
constexpr int WINDOW_EVENT_BASE64_LEN = (sizeof(window_event_t) + 2) / 3 * 4+1;

DEFINE_RPC_OVERRIDE_FUNCTION(JRPCHookHelperEventAPI, "HookHelperEvent");
DEFINE_JRPC_OVERRIDE_FUNCTION(JRPCHookHelperEventAPI);


REGISTER_RPC_EVENT_API_AUTO(JRPCHookHelperEventAPI, HotkeyListUpdate);
DEFINE_REQUEST_RPC_EVENT(JRPCHookHelperEventAPI, HotkeyListUpdate);
bool JRPCHookHelperEventAPI::HotkeyListUpdate(HotKeyList_t& HotKeyListNode)
{
    std::shared_ptr<JsonRPCRequest> req = std::make_shared< JsonRPCRequest>();
    req->SetMethod (HotkeyListUpdateName);

    rapidjson::Writer<FCharBuffer> writer(req->GetParamsBuf());
    DocumentType doc(rapidjson::kArrayType, &ThreadValueAllocator, sizeof(ParseBuffer), &ThreadParseAllocator);
    auto& a = doc.GetAllocator();
    auto fn = [&](this auto&& self,DocumentType& list,const HotKeyList_t& HotKeyListNode)->void {
        for (auto& node : HotKeyListNode) {
            DocumentType obj(rapidjson::kObjectType, &ThreadValueAllocator, sizeof(ParseBuffer), &ThreadParseAllocator);
            obj.AddMember("mod", node.HotKey.mod, a);
            obj.AddMember("keyCode", node.HotKey.key_code, a);
            if (std::holds_alternative<std::string>(node.Child)) {
                auto& str = std::get<std::string>(node.Child);
                obj.AddMember("name", rapidjson::GenericStringRef(str.c_str(), str.size()), a);
            }
            else {
                auto& children = std::get<HotKeyList_t>(node.Child);
                DocumentType childrenNode(rapidjson::kArrayType, &ThreadValueAllocator, sizeof(ParseBuffer), &ThreadParseAllocator);
                self(childrenNode, children);
                obj.AddMember("children", childrenNode, a);
            }
            list.PushBack(obj, a);
        }
        };
    fn(doc,HotKeyListNode);
    if (!doc.Accept(writer)) {
        return false;
    }

    return  processer->SendEvent(req);
}

void JRPCHookHelperEventAPI::OnHotkeyListUpdateRequestRecv(std::shared_ptr<RPCRequest> req)
{
    std::shared_ptr<JsonRPCRequest> jreq = std::dynamic_pointer_cast<JsonRPCRequest>(req);
    HotKeyList_t HotKeyList;
    if (!RecvHotkeyListUpdateDelegate) {
        return;
    }
    auto& buf = jreq->GetParamsBuf();
    buf.Reverse(buf.Length() + simdjson::SIMDJSON_PADDING);
    simdjson::ondemand::document doc = SimdjsonParser.iterate(buf.Data(), buf.Length(), buf.Capacity());

    auto fn = [&](this auto&& self,simdjson::ondemand::array& list, HotKeyList_t& HotKeyList)->bool {
        auto res=list.begin();
        if(res.error() != simdjson::error_code::SUCCESS) {
            return false;
        }
        for (auto it = res.value_unsafe(); it != list.end(); it.operator++()) {
            auto hotkeyNode = *it;
            key_with_modifier_t key;
            if(hotkeyNode.error() != simdjson::error_code::SUCCESS) {
                return false;
            }
            auto keyCode=hotkeyNode["keyCode"].get_int64();
            if (keyCode.error() != simdjson::error_code::SUCCESS) {
                return false;
            }
            key.key_code = keyCode.value_unsafe();
            auto mod=hotkeyNode["mod"].get_uint64();
            if (mod.error() != simdjson::error_code::SUCCESS) {
                return false;
            }
            key.mod = mod.value_unsafe();
            auto name=hotkeyNode["name"].get_string();
            if (name.error() != simdjson::error_code::SUCCESS) {
                auto children = hotkeyNode["children"].get_array();
                if (children.error() != simdjson::error_code::SUCCESS) {
                    return false;
                }
                HotKeyList_t childrenlist;
                if (!self(children.value_unsafe(), childrenlist)) {
                    return false;
                }
                HotKeyList.emplace(std::move(key), std::move(childrenlist));
            }
            else {
                HotKeyList.emplace(std::move(key), std::string(name.value_unsafe()));
            }
        }
        return true;
        };
    auto arr = doc.get_array();
    if(arr.error() != simdjson::error_code::SUCCESS) {
        return;
    }
    fn(arr.value_unsafe(), HotKeyList);

    RecvHotkeyListUpdateDelegate(HotKeyList);
}

REGISTER_RPC_EVENT_API_AUTO(JRPCHookHelperEventAPI, InputStateUpdate);
DEFINE_REQUEST_RPC_EVENT(JRPCHookHelperEventAPI, InputStateUpdate);
bool JRPCHookHelperEventAPI::InputStateUpdate(overlay_ime_event_t& imeEvent)
{
    std::shared_ptr<JsonRPCRequest> req = std::make_shared< JsonRPCRequest>();
    req->SetMethod(InputStateUpdateName);

    rapidjson::Writer<FCharBuffer> writer(req->GetParamsBuf());
    DocumentType doc(rapidjson::kObjectType, &ThreadValueAllocator, sizeof(ParseBuffer), &ThreadParseAllocator);
    auto& a = doc.GetAllocator();
    doc.AddMember("want_visible", imeEvent.want_visible, a);
    doc.AddMember("input_pos_x", imeEvent.input_pos_x, a);
    doc.AddMember("input_pos_y", imeEvent.input_pos_y, a);
    doc.AddMember("input_line_height", imeEvent.input_line_height, a);
    if (!doc.Accept(writer)) {
        return false;
    }

    return  processer->SendEvent(req);
}

void JRPCHookHelperEventAPI::OnInputStateUpdateRequestRecv(std::shared_ptr<RPCRequest> req)
{
    std::shared_ptr<JsonRPCRequest> jreq = std::dynamic_pointer_cast<JsonRPCRequest>(req);
    if (!RecvInputStateUpdateDelegate) {
        return;
    }
    auto& buf = jreq->GetParamsBuf();
    buf.Reverse(buf.Length() + simdjson::SIMDJSON_PADDING);
    simdjson::ondemand::document doc = SimdjsonParser.iterate(buf.Data(), buf.Length(), buf.Capacity());
    auto want_visible = doc["want_visible"].get_bool();
    if (want_visible.error() != simdjson::error_code::SUCCESS) {
        return;
    }
    auto input_pos_x = doc["input_pos_x"].get_uint64();
    if (input_pos_x.error() != simdjson::error_code::SUCCESS) {
        return;
    }
    auto input_pos_y = doc["input_pos_y"].get_uint64();
    if (input_pos_y.error() != simdjson::error_code::SUCCESS) {
        return;
    }
    auto input_line_height = doc["input_line_height"].get_uint64();
    if (input_line_height.error() != simdjson::error_code::SUCCESS) {
        return;
    }
    overlay_ime_event_t imeEvent;
    imeEvent.input_line_height = input_line_height.value_unsafe();
    imeEvent.input_pos_x = input_pos_x.value_unsafe();
    imeEvent.input_pos_y = input_pos_y.value_unsafe();
    imeEvent.want_visible = want_visible.value_unsafe();
    RecvInputStateUpdateDelegate(imeEvent);
}

REGISTER_RPC_EVENT_API_AUTO(JRPCHookHelperEventAPI, ClientSizeUpdate);
DEFINE_REQUEST_RPC_EVENT(JRPCHookHelperEventAPI, ClientSizeUpdate);
bool JRPCHookHelperEventAPI::ClientSizeUpdate(window_resize_event_t& size)
{
    std::shared_ptr<JsonRPCRequest> req = std::make_shared< JsonRPCRequest>();
    req->SetMethod(ClientSizeUpdateName);

    rapidjson::Writer<FCharBuffer> writer(req->GetParamsBuf());
    DocumentType doc(rapidjson::kObjectType, &ThreadValueAllocator, sizeof(ParseBuffer), &ThreadParseAllocator);
    auto& a = doc.GetAllocator();
    doc.AddMember("width", size.width, a);
    doc.AddMember("height", size.height, a);
    if (!doc.Accept(writer)) {
        return false;
    }

    return  processer->SendEvent(req);
}

void JRPCHookHelperEventAPI::OnClientSizeUpdateRequestRecv(std::shared_ptr<RPCRequest> req)
{
    std::shared_ptr<JsonRPCRequest> jreq = std::dynamic_pointer_cast<JsonRPCRequest>(req);
    if (!RecvClientSizeUpdateDelegate) {
        return;
    }
    auto& buf = jreq->GetParamsBuf();
    buf.Reverse(buf.Length() + simdjson::SIMDJSON_PADDING);
    simdjson::ondemand::document doc = SimdjsonParser.iterate(buf.Data(), buf.Length(), buf.Capacity());
    auto width = doc["width"].get_uint64();
    if (width.error() != simdjson::error_code::SUCCESS) {
        return;
    }
    auto height = doc["height"].get_uint64();
    if (height.error() != simdjson::error_code::SUCCESS) {
        return;
    }

    window_resize_event_t revent;
    revent.width = width.value_unsafe();
    revent.height = height.value_unsafe();
    RecvClientSizeUpdateDelegate(revent);
}


REGISTER_RPC_EVENT_API_AUTO(JRPCHookHelperEventAPI, OverlayMouseWheelEvent);
DEFINE_REQUEST_RPC_EVENT(JRPCHookHelperEventAPI, OverlayMouseWheelEvent);
bool JRPCHookHelperEventAPI::OverlayMouseWheelEvent(uint64_t windowId, mouse_wheel_event_t& e)
{
    char base64buf[MOUSE_WHEEL_EVENT_BASE64_LEN + 1];
    size_t base64len = sizeof(base64buf);
    if (!crypto_lib_base64_encode((const uint8_t*)&e, sizeof(mouse_wheel_event_t), (uint8_t*)base64buf, &base64len)) {
        return false;
    }
    std::shared_ptr<JsonRPCRequest> req = std::make_shared< JsonRPCRequest>();
    req->SetMethod(OverlayMouseWheelEventName);

    rapidjson::Writer<FCharBuffer> writer(req->GetParamsBuf());
    DocumentType doc(rapidjson::kObjectType, &ThreadValueAllocator, sizeof(ParseBuffer), &ThreadParseAllocator);
    auto& a = doc.GetAllocator();
    doc.AddMember("windowId", windowId, a);
    doc.AddMember("event", rapidjson::Value(base64buf, base64len,a), a);
    if (!doc.Accept(writer)) {
        return false;
    }

    return  processer->SendEvent(req);
}

void JRPCHookHelperEventAPI::OnOverlayMouseWheelEventRequestRecv(std::shared_ptr<RPCRequest> req)
{
    std::shared_ptr<JsonRPCRequest> jreq = std::dynamic_pointer_cast<JsonRPCRequest>(req);
    if (!RecvOverlayMouseWheelEventDelegate) {
        return;
    }
    auto& buf = jreq->GetParamsBuf();
    buf.Reverse(buf.Length() + simdjson::SIMDJSON_PADDING);
    simdjson::ondemand::document doc = SimdjsonParser.iterate(buf.Data(), buf.Length(), buf.Capacity());
    auto event = doc["event"].get_string();
    if (event.error() != simdjson::error_code::SUCCESS) {
        return;
    }
    auto windowId = doc["windowId"].get_uint64();
    if (windowId.error() != simdjson::error_code::SUCCESS) {
        return;
    }

    mouse_wheel_event_t outEvent;
    size_t outlen = sizeof(mouse_wheel_event_t);
    if (!crypto_lib_base64_decode((uint8_t*)event.value_unsafe().data(), event.value_unsafe().size(), (uint8_t*)&outEvent, &outlen)) {
        return;
    }
    RecvOverlayMouseWheelEventDelegate(windowId.value_unsafe(), outEvent);
}



REGISTER_RPC_EVENT_API_AUTO(JRPCHookHelperEventAPI, OverlayMouseButtonEvent);
DEFINE_REQUEST_RPC_EVENT(JRPCHookHelperEventAPI, OverlayMouseButtonEvent);
bool JRPCHookHelperEventAPI::OverlayMouseButtonEvent(uint64_t windowId, mouse_button_event_t& e)
{
    char base64buf[MOUSE_BOTTON_EVENT_BASE64_LEN + 1];
    size_t base64len = sizeof(base64buf);
    if (!crypto_lib_base64_encode((const uint8_t*)&e, sizeof(mouse_wheel_event_t), (uint8_t*)base64buf, &base64len)) {
        return false;
    }
    std::shared_ptr<JsonRPCRequest> req = std::make_shared< JsonRPCRequest>();
    req->SetMethod(OverlayMouseButtonEventName);

    rapidjson::Writer<FCharBuffer> writer(req->GetParamsBuf());
    DocumentType doc(rapidjson::kObjectType, &ThreadValueAllocator, sizeof(ParseBuffer), &ThreadParseAllocator);
    auto& a = doc.GetAllocator();
    doc.AddMember("windowId", windowId, a);
    doc.AddMember("event", rapidjson::Value(base64buf, base64len, a), a);
    if (!doc.Accept(writer)) {
        return false;
    }

    return  processer->SendEvent(req);
}

void JRPCHookHelperEventAPI::OnOverlayMouseButtonEventRequestRecv(std::shared_ptr<RPCRequest> req)
{
    std::shared_ptr<JsonRPCRequest> jreq = std::dynamic_pointer_cast<JsonRPCRequest>(req);
    if (!RecvOverlayMouseButtonEventDelegate) {
        return;
    }
    auto& buf = jreq->GetParamsBuf();
    buf.Reverse(buf.Length() + simdjson::SIMDJSON_PADDING);
    simdjson::ondemand::document doc = SimdjsonParser.iterate(buf.Data(), buf.Length(), buf.Capacity());
    auto event = doc["event"].get_string();
    if (event.error() != simdjson::error_code::SUCCESS) {
        return;
    }
    auto windowId = doc["windowId"].get_uint64();
    if (windowId.error() != simdjson::error_code::SUCCESS) {
        return;
    }
    mouse_button_event_t outEvent;
    size_t outlen = sizeof(mouse_wheel_event_t);
    if (!crypto_lib_base64_decode((uint8_t*)event.value_unsafe().data(), event.value_unsafe().size(), (uint8_t*)&outEvent, &outlen)) {
        return;
    }
    RecvOverlayMouseButtonEventDelegate(windowId.value_unsafe(), outEvent);
}


REGISTER_RPC_EVENT_API_AUTO(JRPCHookHelperEventAPI, OverlayMouseMotionEvent);
DEFINE_REQUEST_RPC_EVENT(JRPCHookHelperEventAPI, OverlayMouseMotionEvent);
bool JRPCHookHelperEventAPI::OverlayMouseMotionEvent(uint64_t windowId, mouse_motion_event_t& e)
{
    char base64buf[MOUSE_MOTION_EVENT_BASE64_LEN];
    size_t base64len = sizeof(base64buf);
    if (!crypto_lib_base64_encode((const uint8_t*)&e, sizeof(mouse_wheel_event_t), (uint8_t*)base64buf, &base64len)) {
        return false;
    }
    std::shared_ptr<JsonRPCRequest> req = std::make_shared< JsonRPCRequest>();
    req->SetMethod( OverlayMouseMotionEventName);

    rapidjson::Writer<FCharBuffer> writer(req->GetParamsBuf());
    DocumentType doc(rapidjson::kObjectType, &ThreadValueAllocator, sizeof(ParseBuffer), &ThreadParseAllocator);
    auto& a = doc.GetAllocator();
    doc.AddMember("windowId", windowId, a);
    doc.AddMember("event", rapidjson::Value(base64buf, base64len, a), a);
    if (!doc.Accept(writer)) {
        return false;
    }

    return  processer->SendEvent(req);
}

void JRPCHookHelperEventAPI::OnOverlayMouseMotionEventRequestRecv(std::shared_ptr<RPCRequest> req)
{
    std::shared_ptr<JsonRPCRequest> jreq = std::dynamic_pointer_cast<JsonRPCRequest>(req);
    if (!RecvOverlayMouseMotionEventDelegate) {
        return;
    }
    auto& buf = jreq->GetParamsBuf();
    buf.Reverse(buf.Length() + simdjson::SIMDJSON_PADDING);
    simdjson::ondemand::document doc = SimdjsonParser.iterate(buf.Data(), buf.Length(), buf.Capacity());
    auto event = doc["event"].get_string();
    if (event.error() != simdjson::error_code::SUCCESS) {
        return;
    }
    auto windowId = doc["windowId"].get_uint64();
    if (windowId.error() != simdjson::error_code::SUCCESS) {
        return;
    }
    mouse_motion_event_t outEvent;
    size_t outlen = sizeof(mouse_wheel_event_t);
    if (!crypto_lib_base64_decode((uint8_t*)event.value_unsafe().data(), event.value_unsafe().size(), (uint8_t*)&outEvent, &outlen)) {
        return;
    }
    RecvOverlayMouseMotionEventDelegate(windowId.value_unsafe(), outEvent);
}



REGISTER_RPC_EVENT_API_AUTO(JRPCHookHelperEventAPI, OverlayKeyboardEvent);
DEFINE_REQUEST_RPC_EVENT(JRPCHookHelperEventAPI, OverlayKeyboardEvent);
bool JRPCHookHelperEventAPI::OverlayKeyboardEvent(uint64_t windowId, keyboard_event_t& e)
{
    char base64buf[KEYBOARD_EVENT_BASE64_LEN + 1];
    size_t base64len = sizeof(base64buf);
    if (!crypto_lib_base64_encode((const uint8_t*)&e, sizeof(mouse_wheel_event_t), (uint8_t*)base64buf, &base64len)) {
        return false;
    }
    std::shared_ptr<JsonRPCRequest> req = std::make_shared< JsonRPCRequest>();
    req->SetMethod(OverlayKeyboardEventName);

    rapidjson::Writer<FCharBuffer> writer(req->GetParamsBuf());
    DocumentType doc(rapidjson::kObjectType, &ThreadValueAllocator, sizeof(ParseBuffer), &ThreadParseAllocator);
    auto& a = doc.GetAllocator();
    doc.AddMember("windowId", windowId, a);
    doc.AddMember("event", rapidjson::Value(base64buf, base64len, a), a);
    if (!doc.Accept(writer)) {
        return false;
    }

    return  processer->SendEvent(req);
}

void JRPCHookHelperEventAPI::OnOverlayKeyboardEventRequestRecv(std::shared_ptr<RPCRequest> req)
{
    std::shared_ptr<JsonRPCRequest> jreq = std::dynamic_pointer_cast<JsonRPCRequest>(req);
    if (!RecvOverlayKeyboardEventDelegate) {
        return;
    }
    auto& buf = jreq->GetParamsBuf();
    buf.Reverse(buf.Length() + simdjson::SIMDJSON_PADDING);
    simdjson::ondemand::document doc = SimdjsonParser.iterate(buf.Data(), buf.Length(), buf.Capacity());
    auto event = doc["event"].get_string();
    if (event.error() != simdjson::error_code::SUCCESS) {
        return;
    }
    auto windowId = doc["windowId"].get_uint64();
    if (windowId.error() != simdjson::error_code::SUCCESS) {
        return;
    }
    keyboard_event_t outEvent;
    size_t outlen = sizeof(mouse_wheel_event_t);
    if (!crypto_lib_base64_decode((uint8_t*)event.value_unsafe().data(), event.value_unsafe().size(), (uint8_t*)&outEvent, &outlen)) {
        return;
    }
    RecvOverlayKeyboardEventDelegate(windowId.value_unsafe(), outEvent);
}


REGISTER_RPC_EVENT_API_AUTO(JRPCHookHelperEventAPI, OverlayCharEvent);
DEFINE_REQUEST_RPC_EVENT(JRPCHookHelperEventAPI, OverlayCharEvent);
bool JRPCHookHelperEventAPI::OverlayCharEvent(uint64_t windowId, overlay_char_event_t& e)
{
    std::shared_ptr<JsonRPCRequest> req = std::make_shared< JsonRPCRequest>();
    req->SetMethod(OverlayCharEventName);

    rapidjson::Writer<FCharBuffer> writer(req->GetParamsBuf());
    DocumentType doc(rapidjson::kObjectType, &ThreadValueAllocator, sizeof(ParseBuffer), &ThreadParseAllocator);
    auto& a = doc.GetAllocator();
    doc.AddMember("windowId", windowId, a);
    doc.AddMember("str", rapidjson::Value(e.char_buf, e.num, a), a);
    if (!doc.Accept(writer)) {
        return false;
    }

    return  processer->SendEvent(req);
}

void JRPCHookHelperEventAPI::OnOverlayCharEventRequestRecv(std::shared_ptr<RPCRequest> req)
{
    std::shared_ptr<JsonRPCRequest> jreq = std::dynamic_pointer_cast<JsonRPCRequest>(req);
    if (!RecvOverlayCharEventDelegate) {
        return;
    }
    auto& buf = jreq->GetParamsBuf();
    buf.Reverse(buf.Length() + simdjson::SIMDJSON_PADDING);
    simdjson::ondemand::document doc = SimdjsonParser.iterate(buf.Data(), buf.Length(), buf.Capacity());
    auto str = doc["str"].get_string();
    if (str.error() != simdjson::error_code::SUCCESS) {
        return;
    }
    auto windowId = doc["windowId"].get_uint64();
    if (windowId.error() != simdjson::error_code::SUCCESS) {
        return;
    }
    overlay_char_event_t outEvent;
    outEvent.char_buf = str.value_unsafe().data();
    outEvent.num = str.value_unsafe().size();

    RecvOverlayCharEventDelegate(windowId.value_unsafe(), outEvent);
}


REGISTER_RPC_EVENT_API_AUTO(JRPCHookHelperEventAPI, OverlayWindowEvent);
DEFINE_REQUEST_RPC_EVENT(JRPCHookHelperEventAPI, OverlayWindowEvent);
bool JRPCHookHelperEventAPI::OverlayWindowEvent(uint64_t windowId, window_event_t& e)
{
    char base64buf[WINDOW_EVENT_BASE64_LEN + 1];
    size_t base64len = sizeof(base64buf);
    if (!crypto_lib_base64_encode((const uint8_t*)&e, sizeof(mouse_wheel_event_t), (uint8_t*)base64buf, &base64len)) {
        return false;
    }
    std::shared_ptr<JsonRPCRequest> req = std::make_shared< JsonRPCRequest>();
    req->SetMethod(OverlayWindowEventName);

    rapidjson::Writer<FCharBuffer> writer(req->GetParamsBuf());
    DocumentType doc(rapidjson::kObjectType, &ThreadValueAllocator, sizeof(ParseBuffer), &ThreadParseAllocator);
    auto& a = doc.GetAllocator();
    doc.AddMember("windowId", windowId, a);
    doc.AddMember("event", rapidjson::Value(base64buf, base64len, a), a);
    if (!doc.Accept(writer)) {
        return false;
    }

    return  processer->SendEvent(req);
}

void JRPCHookHelperEventAPI::OnOverlayWindowEventRequestRecv(std::shared_ptr<RPCRequest> req)
{
    std::shared_ptr<JsonRPCRequest> jreq = std::dynamic_pointer_cast<JsonRPCRequest>(req);
    if (!RecvOverlayWindowEventDelegate) {
        return;
    }
    auto& buf = jreq->GetParamsBuf();
    buf.Reverse(buf.Length() + simdjson::SIMDJSON_PADDING);
    simdjson::ondemand::document doc = SimdjsonParser.iterate(buf.Data(), buf.Length(), buf.Capacity());
    auto event = doc["event"].get_string();
    if (event.error() != simdjson::error_code::SUCCESS) {
        return;
    }
    auto windowId = doc["windowId"].get_uint64();
    if (windowId.error() != simdjson::error_code::SUCCESS) {
        return;
    }
    window_event_t outEvent;
    size_t outlen = sizeof(mouse_wheel_event_t);
    if (!crypto_lib_base64_decode((uint8_t*)event.value_unsafe().data(), event.value_unsafe().size(), (uint8_t*)&outEvent, &outlen)) {
        return;
    }
    RecvOverlayWindowEventDelegate(windowId.value_unsafe(), outEvent);
}