#include "RPC/JrpcHookHelper.h"
#include "JrpcHookHelperInternal.h"

#include <RPC/message_common.h>
#include <jrpc_parser.h>
#include <simdjson.h>
#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

#include <memory>
#include <stdint.h>
#include <shared_mutex>
#include <map>
#include <string>
#include <chrono>
#include <list>


DEFINE_RPC_OVERRIDE_FUNCTION(JRPCHookHelperAPI, "HookHelper");
DEFINE_JRPC_OVERRIDE_FUNCTION(JRPCHookHelperAPI);

REGISTER_RPC_API_AUTO(JRPCHookHelperAPI, ConnectToHost);
DEFINE_REQUEST_RPC(JRPCHookHelperAPI, ConnectToHost);
RPCHandle_t JRPCHookHelperAPI::ConnectToHost(uint64_t processId, std::string_view commandline, TConnectToHostDelegate inDelegate, TRPCErrorDelegate errDelegate)
{
    std::shared_ptr<JsonRPCRequest> req = std::make_shared< JsonRPCRequest>();
    req->SetMethod(ConnectToHostName);

    rapidjson::Writer<FCharBuffer> writer(req->GetParamsBuf());
    DocumentType doc(rapidjson::kObjectType, &ThreadValueAllocator, sizeof(ParseBuffer), &ThreadParseAllocator);
    auto& a = doc.GetAllocator();
    doc.AddMember("processId", processId, a);
    doc.AddMember("commandline", rapidjson::Value(commandline.data(), commandline.size(), a), a);
    if (!doc.Accept(writer)) {
        return NullHandle;
    }

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

    rapidjson::Writer<FCharBuffer> writer(response->GetResultBuf());
    DocumentType doc(rapidjson::kObjectType, &ThreadValueAllocator, sizeof(ParseBuffer), &ThreadParseAllocator);
    if (!doc.Accept(writer)) {
        return false;
    }

    return processer->SendResponse(handle, response);
}

void JRPCHookHelperAPI::OnConnectToHostRequestRecv(std::shared_ptr<RPCRequest> req)
{
    std::shared_ptr<JsonRPCRequest> jreq = std::dynamic_pointer_cast<JsonRPCRequest>(req);
    auto& buf = jreq->GetParamsBuf();
    buf.Reverse(buf.Length() + simdjson::SIMDJSON_PADDING);
    simdjson::ondemand::document doc = SimdjsonParser.iterate(buf.Data(), buf.Length(), buf.Size());
    auto processId = doc["processId"].get_uint64();
    if (processId.error() != simdjson::error_code::SUCCESS) {
        return;
    }
    auto commandline = doc["commandline"].get_string();
    if (commandline.error() != simdjson::error_code::SUCCESS) {
        return;
    }

    RecvConnectToHostDelegate(
        RPCHandle_t(req->GetID()), 
        processId.value_unsafe(), 
        commandline.value_unsafe()
    );
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
RPCHandle_t JRPCHookHelperAPI::AddWindow(uint64_t windowID, std::string_view sharedMemName, TAddWindowDelegate inDelegate, TRPCErrorDelegate errDelegate)
{
    std::shared_ptr<JsonRPCRequest> req = std::make_shared< JsonRPCRequest>();
    req->SetMethod(AddWindowName);

    rapidjson::Writer<FCharBuffer> writer(req->GetParamsBuf());
    DocumentType doc(rapidjson::kObjectType, &ThreadValueAllocator, sizeof(ParseBuffer), &ThreadParseAllocator);
    auto& a = doc.GetAllocator();
    doc.AddMember("windowID", windowID, a);
    doc.AddMember("sharedMemName", rapidjson::Value(sharedMemName.data(), sharedMemName.size(), a), a);
    if (!doc.Accept(writer)) {
        return NullHandle;
    }

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

    rapidjson::Writer<FCharBuffer> writer(response->GetResultBuf());
    DocumentType doc(rapidjson::kObjectType, &ThreadValueAllocator, sizeof(ParseBuffer), &ThreadParseAllocator);
    if (!doc.Accept(writer)) {
        return false;
    }

    return processer->SendResponse(handle, response);
}

void JRPCHookHelperAPI::OnAddWindowRequestRecv(std::shared_ptr<RPCRequest> req)
{
    std::shared_ptr<JsonRPCRequest> jreq = std::dynamic_pointer_cast<JsonRPCRequest>(req);
    auto& buf = jreq->GetParamsBuf();
    buf.Reverse(buf.Length() + simdjson::SIMDJSON_PADDING);
    simdjson::ondemand::document doc = SimdjsonParser.iterate(buf.Data(), buf.Length(), buf.Size());
    auto windowID = doc["windowID"].get_uint64();
    if (windowID.error() != simdjson::error_code::SUCCESS) {
        return;
    }
    auto sharedMemName = doc["sharedMemName"].get_string();
    if (sharedMemName.error() != simdjson::error_code::SUCCESS) {
        return;
    }
    if (RecvAddWindowDelegate)
        RecvAddWindowDelegate(
            RPCHandle_t(req->GetID()),
            windowID.value_unsafe(),
            sharedMemName.value_unsafe()
        );
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

    rapidjson::Writer<FCharBuffer> writer(req->GetParamsBuf());
    DocumentType doc(rapidjson::kObjectType, &ThreadValueAllocator, sizeof(ParseBuffer), &ThreadParseAllocator);
    auto& a = doc.GetAllocator();
    doc.AddMember("windowID", windowID, a);
    if (!doc.Accept(writer)) {
        return NullHandle;
    }

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

    rapidjson::Writer<FCharBuffer> writer(response->GetResultBuf());
    DocumentType doc(rapidjson::kObjectType, &ThreadValueAllocator, sizeof(ParseBuffer), &ThreadParseAllocator);
    if (!doc.Accept(writer)) {
        return false;
    }

    return processer->SendResponse(handle, response);
}

void JRPCHookHelperAPI::OnRemoveWindowRequestRecv(std::shared_ptr<RPCRequest> req)
{
    std::shared_ptr<JsonRPCRequest> jreq = std::dynamic_pointer_cast<JsonRPCRequest>(req);
    auto& buf = jreq->GetParamsBuf();
    buf.Reverse(buf.Length() + simdjson::SIMDJSON_PADDING);
    simdjson::ondemand::document doc = SimdjsonParser.iterate(buf.Data(), buf.Length(), buf.Size());
    auto windowID = doc["windowID"].get_uint64();
    if (windowID.error() != simdjson::error_code::SUCCESS) {
        return;
    }
    if (RecvRemoveWindowDelegate) {
        RecvRemoveWindowDelegate(RPCHandle_t(req->GetID()), windowID.value_unsafe());
    }
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

    rapidjson::Writer<FCharBuffer> writer(req->GetParamsBuf());
    DocumentType doc(rapidjson::kObjectType, &ThreadValueAllocator, sizeof(ParseBuffer), &ThreadParseAllocator);
    auto& a = doc.GetAllocator();
    doc.AddMember("windowID", windowID, a);
    if (!doc.Accept(writer)) {
        return NullHandle;
    }

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

    rapidjson::Writer<FCharBuffer> writer(response->GetResultBuf());
    DocumentType doc(rapidjson::kObjectType, &ThreadValueAllocator, sizeof(ParseBuffer), &ThreadParseAllocator);
    if (!doc.Accept(writer)) {
        return false;
    }

    return processer->SendResponse(handle, response);
}

void JRPCHookHelperAPI::OnUpdateWindowTextureRequestRecv(std::shared_ptr<RPCRequest> req)
{
    std::shared_ptr<JsonRPCRequest> jreq = std::dynamic_pointer_cast<JsonRPCRequest>(req);
    auto& buf = jreq->GetParamsBuf();
    buf.Reverse(buf.Length() + simdjson::SIMDJSON_PADDING);
    simdjson::ondemand::document doc = SimdjsonParser.iterate(buf.Data(), buf.Length(), buf.Size());
    auto windowID = doc["windowID"].get_uint64();
    if (windowID.error() != simdjson::error_code::SUCCESS) {
        return;
    }
    if (RecvUpdateWindowTextureDelegate) {
        RecvUpdateWindowTextureDelegate(RPCHandle_t(req->GetID()), windowID.value_unsafe());
    }
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