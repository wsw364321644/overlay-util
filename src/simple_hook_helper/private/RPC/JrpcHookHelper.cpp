#include "RPC/JrpcHookHelper.h"

#include <jrpc_parser.h>
#include <memory>
#include <stdint.h>
#include <shared_mutex>
#include <map>
#include <string>
#include <chrono>
#include <list>
#include <RPC/message_common.h>

DEFINE_RPC_OVERRIDE_FUNCTION(JRPCHookHelperAPI, "HookHelper");
DEFINE_JRPC_OVERRIDE_FUNCTION(JRPCHookHelperAPI);

REGISTER_RPC_API_AUTO(JRPCHookHelperAPI, ConnectToHost);
DEFINE_REQUEST_RPC(JRPCHookHelperAPI, ConnectToHost);
RPCHandle_t JRPCHookHelperAPI::ConnectToHost(uint64_t processId, const char* commandline, TConnectToHostDelegate inDelegate, TRPCErrorDelegate errDelegate)
{
    std::shared_ptr<JsonRPCRequest> req = std::make_shared< JsonRPCRequest>();
    req->SetMethod(ConnectToHostName);

    nlohmann::json obj = nlohmann::json::object();
    obj["processId"] = processId;
    obj["commandline"] = commandline;
    req->SetParams(obj.dump());

    auto handle = processer->SendRequest(req);
    if (handle.IsValid()) {
        AddConnectToHostSendDelagate(req->GetID(), inDelegate, errDelegate);
    }
    return handle;
}

bool JRPCHookHelperAPI::RespondConnectToHost(RPCHandle_t handle)
{
    std::shared_ptr<JsonRPCResponse> response = std::make_shared< JsonRPCResponse>();
    response->SetError(false);

    nlohmann::json doc = nlohmann::json();
    response->SetResult(doc.dump());
    return processer->SendResponse(handle, response);
}

void JRPCHookHelperAPI::OnConnectToHostRequestRecv(std::shared_ptr<RPCRequest> req)
{
    std::shared_ptr<JsonRPCRequest> jreq = std::dynamic_pointer_cast<JsonRPCRequest>(req);
    auto doc = GetParamsNlohmannJson(*jreq);

    RecvConnectToHostDelegate(RPCHandle_t(req->GetID()), doc["processId"].get_ref<nlohmann::json::number_integer_t&>(), doc["commandline"].get_ref<nlohmann::json::string_t&>().c_str());
}

void JRPCHookHelperAPI::OnConnectToHostResponseRecv(std::shared_ptr<RPCResponse>resp, std::shared_ptr<RPCRequest>req)
{
    auto id = req->GetID();
    if (!HasConnectToHostSendDelagate(id)) {
        return;
    }
    std::shared_ptr<JsonRPCResponse> jresp = std::dynamic_pointer_cast<JsonRPCResponse>(resp);

    if (jresp->IsError()) {
        TriggerConnectToHostSendErrorDelegate(id, resp->ErrorCode, resp->GetErrorMsg().data(), resp->GetErrorData().data());
    }
    else {
        TriggerConnectToHostSendDelegate(id);
    }
    RemoveConnectToHostSendDelagate(id);
}

REGISTER_RPC_API_AUTO(JRPCHookHelperAPI, AddWindow);
DEFINE_REQUEST_RPC(JRPCHookHelperAPI, AddWindow);
RPCHandle_t JRPCHookHelperAPI::AddWindow(uint64_t windowID, const char* sharedMemName, TAddWindowDelegate inDelegate, TRPCErrorDelegate errDelegate)
{
    std::shared_ptr<JsonRPCRequest> req = std::make_shared< JsonRPCRequest>();
    req->SetMethod(AddWindowName);

    nlohmann::json obj = nlohmann::json::object();
    obj["windowID"] = windowID;
    obj["sharedMemName"] = sharedMemName;
    req->SetParams(obj.dump());

    auto handle = processer->SendRequest(req);
    if (handle.IsValid()) {
        AddAddWindowSendDelagate(req->GetID(), inDelegate, errDelegate);
    }
    return handle;
}

bool JRPCHookHelperAPI::RespondAddWindow(RPCHandle_t handle)
{
    std::shared_ptr<JsonRPCResponse> response = std::make_shared< JsonRPCResponse>();
    response->SetError(false);

    nlohmann::json doc = nlohmann::json();
    response->SetResult(doc.dump());
    return processer->SendResponse(handle, response);
}

void JRPCHookHelperAPI::OnAddWindowRequestRecv(std::shared_ptr<RPCRequest> req)
{
    std::shared_ptr<JsonRPCRequest> jreq = std::dynamic_pointer_cast<JsonRPCRequest>(req);
    auto doc = GetParamsNlohmannJson(*jreq);
    if (RecvAddWindowDelegate)
        RecvAddWindowDelegate(RPCHandle_t(req->GetID()), doc["windowID"].get_ref<nlohmann::json::number_integer_t&>(), doc["sharedMemName"].get_ref<nlohmann::json::string_t&>().c_str());
}

void JRPCHookHelperAPI::OnAddWindowResponseRecv(std::shared_ptr<RPCResponse>resp, std::shared_ptr<RPCRequest>req)
{
    auto id = req->GetID();
    if (!HasAddWindowSendDelagate(id)) {
        return;
    }
    std::shared_ptr<JsonRPCResponse> jresp = std::dynamic_pointer_cast<JsonRPCResponse>(resp);

    if (jresp->IsError()) {
        TriggerAddWindowSendErrorDelegate(id, resp->ErrorCode, resp->GetErrorMsg().data(), resp->GetErrorData().data());
    }
    else {
        TriggerAddWindowSendDelegate(id);
    }
    RemoveAddWindowSendDelagate(id);
}



REGISTER_RPC_API_AUTO(JRPCHookHelperAPI, RemoveWindow);
DEFINE_REQUEST_RPC(JRPCHookHelperAPI, RemoveWindow);
RPCHandle_t JRPCHookHelperAPI::RemoveWindow(uint64_t windowID,TRemoveWindowDelegate inDelegate, TRPCErrorDelegate errDelegate)
{
    std::shared_ptr<JsonRPCRequest> req = std::make_shared< JsonRPCRequest>();
    req->SetMethod(RemoveWindowName);

    nlohmann::json obj = nlohmann::json::object();
    obj["windowID"] = windowID;
    req->SetParams(obj.dump());

    auto handle = processer->SendRequest(req);
    if (handle.IsValid()) {
        AddRemoveWindowSendDelagate(req->GetID(), inDelegate, errDelegate);
    }
    return handle;
}

bool JRPCHookHelperAPI::RespondRemoveWindow(RPCHandle_t handle)
{
    std::shared_ptr<JsonRPCResponse> response = std::make_shared< JsonRPCResponse>();
    response->SetError(false);

    nlohmann::json doc = nlohmann::json();
    response->SetResult(doc.dump());
    return processer->SendResponse(handle, response);
}

void JRPCHookHelperAPI::OnRemoveWindowRequestRecv(std::shared_ptr<RPCRequest> req)
{
    std::shared_ptr<JsonRPCRequest> jreq = std::dynamic_pointer_cast<JsonRPCRequest>(req);
    auto doc = GetParamsNlohmannJson(*jreq);
    if(RecvRemoveWindowDelegate)
        RecvRemoveWindowDelegate(RPCHandle_t(req->GetID()), doc["windowID"].get_ref<nlohmann::json::number_integer_t&>());
}

void JRPCHookHelperAPI::OnRemoveWindowResponseRecv(std::shared_ptr<RPCResponse>resp, std::shared_ptr<RPCRequest>req)
{
    auto id = req->GetID();
    if (!HasRemoveWindowSendDelagate(id)) {
        return;
    }
    std::shared_ptr<JsonRPCResponse> jresp = std::dynamic_pointer_cast<JsonRPCResponse>(resp);

    if (jresp->IsError()) {
        TriggerRemoveWindowSendErrorDelegate(id, resp->ErrorCode, resp->GetErrorMsg().data(), resp->GetErrorData().data());
    }
    else {
        TriggerRemoveWindowSendDelegate(id);
    }
    RemoveRemoveWindowSendDelagate(id);
}


REGISTER_RPC_API_AUTO(JRPCHookHelperAPI, UpdateWindowTexture);
DEFINE_REQUEST_RPC(JRPCHookHelperAPI, UpdateWindowTexture);
RPCHandle_t JRPCHookHelperAPI::UpdateWindowTexture(uint64_t windowID, TUpdateWindowTextureDelegate inDelegate, TRPCErrorDelegate errDelegate)
{
    std::shared_ptr<JsonRPCRequest> req = std::make_shared< JsonRPCRequest>();
    req->SetMethod(UpdateWindowTextureName);

    nlohmann::json obj = nlohmann::json::object();
    obj["windowID"] = windowID;
    req->SetParams(obj.dump());

    auto handle = processer->SendRequest(req);
    if (handle.IsValid()) {
        AddUpdateWindowTextureSendDelagate(req->GetID(), inDelegate, errDelegate);
    }
    return handle;
}

bool JRPCHookHelperAPI::RespondUpdateWindowTexture(RPCHandle_t handle)
{
    std::shared_ptr<JsonRPCResponse> response = std::make_shared< JsonRPCResponse>();
    response->SetError(false);

    nlohmann::json doc = nlohmann::json();
    response->SetResult(doc.dump());
    return processer->SendResponse(handle, response);
}

void JRPCHookHelperAPI::OnUpdateWindowTextureRequestRecv(std::shared_ptr<RPCRequest> req)
{
    std::shared_ptr<JsonRPCRequest> jreq = std::dynamic_pointer_cast<JsonRPCRequest>(req);
    auto doc = GetParamsNlohmannJson(*jreq);
    if (RecvUpdateWindowTextureDelegate)
        RecvUpdateWindowTextureDelegate(RPCHandle_t(req->GetID()), doc["windowID"].get_ref<nlohmann::json::number_integer_t&>());
}

void JRPCHookHelperAPI::OnUpdateWindowTextureResponseRecv(std::shared_ptr<RPCResponse>resp, std::shared_ptr<RPCRequest>req)
{
    auto id = req->GetID();
    if (!HasUpdateWindowTextureSendDelagate(id)) {
        return;
    }
    std::shared_ptr<JsonRPCResponse> jresp = std::dynamic_pointer_cast<JsonRPCResponse>(resp);

    if (jresp->IsError()) {
        TriggerUpdateWindowTextureSendErrorDelegate(id, resp->ErrorCode, resp->GetErrorMsg().data(), resp->GetErrorData().data());
    }
    else {
        TriggerUpdateWindowTextureSendDelegate(id);
    }
    RemoveUpdateWindowTextureSendDelagate(id);
}