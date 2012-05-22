/**
 * @file ICECandidate.cpp
 *
 *
 */

/******************************************************************************
 * Copyright 2009,2012 Qualcomm Innovation Center, Inc.
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 ******************************************************************************/

#include <qcc/platform.h>
#include <qcc/atomic.h>

#include <ICEStream.h>
#include <ICECandidate.h>
#include <StunActivity.h>
#include <Component.h>
#include <StunAttribute.h>
#include <ICESession.h>

#include <map>

using namespace qcc;

/** @internal */
#define QCC_MODULE "ICECANDIDATE"

namespace ajn {

ICECandidate::ICECandidate(ICECandidate::ICECandidateType type, IPEndpoint endPoint, IPEndpoint base, Component* component, SocketType transportProtocol, StunActivity* stunActivity,
                           String interfaceName)
    : type(type), priority(), endPoint(endPoint), base(base), mappedAddress(), grantedAllocationLifetimeSecs(),
    foundation(), component(component), transportProtocol(transportProtocol), stunActivity(stunActivity),
    permissionStunActivity(), terminating(false), sharedStunRelayedCandidate(), sharedStunServerReflexiveCandidate(),
    sharedStunPeerReflexiveCandidate(), listenerThread(NULL), refs(0), InterfaceName(interfaceName)
{
    QCC_DbgTrace(("ICECandidate::ICECandidate1(%p, type=%d)", this, type));
    stunActivity->SetCandidate(this);
}

ICECandidate::ICECandidate(IPEndpoint endPoint, IPEndpoint base, IPEndpoint mappedAddress, uint32_t grantedAllocationLifetimeSecs,
                           Component* component, SocketType transportProtocol, StunActivity* stunActivity, StunActivity* permissionStunActivity)
    : type(ICECandidate::Relayed_Candidate), priority(), endPoint(endPoint), base(base), mappedAddress(mappedAddress),
    grantedAllocationLifetimeSecs(grantedAllocationLifetimeSecs), foundation(), component(component), transportProtocol(transportProtocol),
    stunActivity(stunActivity), permissionStunActivity(permissionStunActivity),
    terminating(false), sharedStunRelayedCandidate(), sharedStunServerReflexiveCandidate(), sharedStunPeerReflexiveCandidate(), listenerThread(NULL)
{
    QCC_DbgTrace(("ICECandidate::~ICECandidate2(%p, relayed)", this));
    stunActivity->SetCandidate(this);
    permissionStunActivity->SetCandidate(this);
}

ICECandidate::ICECandidate(ICECandidate::ICECandidateType type, IPEndpoint endPoint, Component* component, SocketType transportProtocol, uint32_t priority, String foundation)
    : type(type), priority(priority), endPoint(endPoint), base(), mappedAddress(),
    grantedAllocationLifetimeSecs(), foundation(foundation), component(component), transportProtocol(transportProtocol), stunActivity(),
    permissionStunActivity(), terminating(false), sharedStunRelayedCandidate(), sharedStunServerReflexiveCandidate(), sharedStunPeerReflexiveCandidate(), listenerThread(NULL)
{
    QCC_DbgTrace(("ICECandidate::~ICECandidate3(%p, type=%d)", this, type));
    if (stunActivity) {
        stunActivity->SetCandidate(this);
    }
}

ICECandidate::~ICECandidate(void)
{
    QCC_DbgTrace(("ICECandidate::~ICECandidate(%p)", this));
    terminating = true;

    if (NULL != listenerThread && Thread::GetThread() != listenerThread) {
        // Wait on listener thread.
        QCC_DbgPrintf(("Stopping listener thread 0x%x", listenerThread));

        listenerThread->Stop();
        listenerThread->Join();
        delete listenerThread;
        listenerThread = NULL;
    }
}

void ICECandidate::IncRef()
{
    qcc::IncrementAndFetch(&refs);
}

void ICECandidate::DecRef()
{
    if (qcc::DecrementAndFetch(&refs) <= 0) {
        delete this;
    }
}

QStatus ICECandidate::StartListener(void)
{
    listenerThread = new Thread("ListenerThreadStub", ListenerThreadStub);

    // Start the thread which will listen for responses, ICE checks
    return (listenerThread->Start(this));
}


QStatus ICECandidate::StopCheckListener(void)
{
    QStatus status = ER_OK;

    QCC_DbgPrintf(("ICECandidate::StopCheckListener()"));

    // Notify checkListener thread to quit.
    terminating = true;

    return status;
}

void ICECandidate::AwaitRequestsAndResponses(void)
{
    // This method runs only in the instance of a Host Candidate, but
    // because the host candidate's stun object is shared by reflexive and
    // relayed candidates for a component, we need to be prepared
    // to receive a message for any of these.
    assert(NULL != stunActivity);
    assert(NULL != stunActivity->stun);

    // This timer provides an upper limit on the time to shutdown.
    // Decreasing it makes shutdown more responsive, at the expense
    // of busy loop polling. It has nothing to do with
    // retransmit timeouts, pacing, etc.
    uint32_t worstCaseShudownTimeoutMsec = 5000;
    bool signaledStop = false;

    QStatus status = ER_OK;
    while (!terminating && !signaledStop) {
        // block until receive data is ready, or timeout
        switch (status = ReadReceivedMessage(worstCaseShudownTimeoutMsec)) {
        case ER_TIMEOUT:
            // Timer has expired. Go around for another try.
            break;

        case ER_OK:
            // Message was processed. Go around for next one.
            break;

        case ER_STOPPING_THREAD:
            signaledStop = true;
            break;

        default:
            QCC_LogError(status, ("ReadReceivedMessage"));
            break;
        }
    }

    QCC_DbgPrintf(("AwaitCheckRequestsAndResponses terminating"));
    // Thread terminates.
}

// Section 7.2.1 draft-ietf-mmusic-ice-19
QStatus ICECandidate::ReadReceivedMessage(uint32_t timeoutMsec)
{
    // This method runs in the instance of a Host Candidate, but
    // because the host candidate's stun object is shared by server
    // reflexive and relayed candidates for a component, we need to be prepared
    // to receive a message for any of these.

    QStatus status = ER_OK;
    ICECandidatePair::CheckStatus checkStatus = ICECandidatePair::CheckGenericFailed;   // Assume the worst.
    String username;

    IPEndpoint mappedAddress;
    IPEndpoint remote;
    bool receivedMsgWasRelayed;

    StunMessage::const_iterator stunAttrIt;
    StunTransactionID requestTransID;
    bool ICEcontrollingRequest = false;
    bool useCandidateRequest = false;
    //uint64_t controlTieBreaker = 0;
    uint32_t requestPriority = 0;
    StunTransactionID tid;

    component->GetICEStream()->GetSession()->Lock();

    StunMessage msg(component->GetICEStream()->GetSession()->GetRemoteInitiatedCheckUsername(),
                    component->GetICEStream()->GetSession()->GetRemoteInitiatedCheckHmacKey(),
                    component->GetICEStream()->GetSession()->GetRemoteInitiatedCheckHmacKeyLength());

    component->GetICEStream()->GetSession()->Unlock();

    status = stunActivity->stun->RecvStunMessage(msg, remote.addr, remote.port, receivedMsgWasRelayed, timeoutMsec);

    if (ER_OK != status) {
        QCC_DbgPrintf(("ReadReceivedMessage status = %d", status));
        return status;
    }
#if !defined(NDEBUG)
    QCC_DbgPrintf(("ReadRxMsg status=%d, class=%s,  base %s:%d %s from %s:%d",
                   status, msg.MessageClassToString(msg.GetTypeClass()).c_str(),
                   base.addr.ToString().c_str(), base.port,
                   receivedMsgWasRelayed ? "relayed" : "",
                   remote.addr.ToString().c_str(), remote.port));
#endif
    component->GetICEStream()->GetSession()->Lock();

    // ToDo: If the message is a request and does not contain both a MESSAGE-INTEGRITY and a
    // USERNAME attribute: reject the request with an error response.  This response MUST use an error code
    // of 400 (Bad Request).

    //ToDo: (RFC 5389 10.1.2) If the message is a request and fails MESSAGE-INTEGRITY
    // the server MUST reject the request with error response 401 (Unauthorized).

    msg.GetTransactionID(tid);

    // We don't know if this is a request or response yet, but assume it is a response
    // and try to match up transaction in list of checks that we have sent.
    CheckRetry* checkRetry = component->GetCheckRetryByTransaction(tid);
    if (NULL != checkRetry) {
        QCC_DbgPrintf(("TID: %s, %s", tid.ToString().c_str(),
                       (msg.GetTypeClass() == STUN_MSG_RESPONSE_CLASS) ? "Check Response matches" : "Unlikely but can happen!!!"));
    }

    // We don't know if this is a request or response yet, but assume it is a response
    // and try to match up transaction in list of non-checks (Allocate refresh, etc) that we have sent.
    Retransmit* retransmit = component->GetRetransmitByTransaction(tid);
    if (NULL != retransmit) {
        QCC_DbgPrintf(("TID: %s, %s", tid.ToString().c_str(),
                       ((msg.GetTypeClass() == STUN_MSG_RESPONSE_CLASS) || (msg.GetTypeClass() == STUN_MSG_ERROR_CLASS))
                       ? "NonCheck Response matches" : "Duplicate TID!!!"));
    }

    if (NULL == checkRetry && NULL == retransmit) {
        QCC_DbgPrintf(("TID: %s, Unknown %s", tid.ToString().c_str(),
                       (msg.GetTypeClass() == STUN_MSG_REQUEST_CLASS) ? "Request" : "Response"));
    }


    ICECandidate* relayedCandidate = NULL;
    IPEndpoint reflexive;
    IPEndpoint relayed;
    uint32_t grantedAllocationLifetimeSecs = 0;

    // iterate thru message looking for attributes
    for (stunAttrIt = msg.Begin(); stunAttrIt != msg.End(); ++stunAttrIt) {
        switch ((*stunAttrIt)->GetType()) {
        case STUN_ATTR_XOR_MAPPED_ADDRESS: {
            const StunAttributeXorMappedAddress& sa = *reinterpret_cast<StunAttributeXorMappedAddress*>(*stunAttrIt);

            IPEndpoint base;
            stunActivity->stun->GetLocalAddress(base.addr, base.port);

            // Should only appear in a response to our earlier (outbound) check.
            // To later determine peer-reflexive candidate...
            sa.GetAddress(mappedAddress.addr, mappedAddress.port);

            sa.GetAddress(reflexive.addr, reflexive.port);

            if (ICESession::ICEGatheringCandidates ==
                component->GetICEStream()->GetSession()->GetState()) {
                if (relayedCandidate) {
                    relayedCandidate->SetMappedAddress(reflexive);
                }

                // Discard if reflexive is identical to host
                if (base.addr != reflexive.addr) {
                    StunActivity* reflexiveCandidateStunActivity =
                        new StunActivity(stunActivity->stun);

                    stunActivity->stun->GetComponent()->AddToStunActivityList(
                        reflexiveCandidateStunActivity);

                    sharedStunServerReflexiveCandidate = new ICECandidate(
                        ICECandidate::ServerReflexive_Candidate,
                        reflexive, base, stunActivity->stun->GetComponent(),
                        stunActivity->stun->GetSocketType(),
                        reflexiveCandidateStunActivity, InterfaceName);

                    // store server_reflexive candidate (reuse host candidate's stun object)
                    stunActivity->stun->GetComponent()->AddCandidate(sharedStunServerReflexiveCandidate);
                }
            }
            // cease retries
            if (NULL != retransmit) {
                retransmit->SetState(Retransmit::ReceivedSuccessResponse);
            }
            break;
        }

        case STUN_ATTR_XOR_PEER_ADDRESS:
        {

            break;
        }

        case STUN_ATTR_XOR_RELAYED_ADDRESS: {
            const StunAttributeXorRelayedAddress& sa = *reinterpret_cast<StunAttributeXorRelayedAddress*>(*stunAttrIt);

            IPEndpoint host;
            stunActivity->stun->GetLocalAddress(host.addr, host.port);
            sa.GetAddress(relayed.addr, relayed.port);

            if (ICESession::ICEGatheringCandidates ==
                component->GetICEStream()->GetSession()->GetState()) {
                // Discard if relayed is identical to host
                if (relayed.addr != host.addr) {
                    // (Reuse host candidate's stun object.)

                    //  Maintain count of retries and timeouts as we perform Allocate
                    // _refresh_ requests to the TURN server for this relayed candidate.
                    StunActivity* relayedCandidateStunActivity = new StunActivity(stunActivity->stun);

                    stunActivity->stun->GetComponent()->AddToStunActivityList(
                        relayedCandidateStunActivity);

                    //  Maintain count of retries and timeouts as we perform Permission
                    // _refresh_ requests to the TURN server for this relayed candidate.
                    StunActivity* permissionStunActivity = new StunActivity(stunActivity->stun);

                    stunActivity->stun->GetComponent()->AddToStunActivityList(
                        permissionStunActivity);

                    relayedCandidate = new ICECandidate(relayed, relayed, reflexive, grantedAllocationLifetimeSecs,
                                                        stunActivity->stun->GetComponent(), stunActivity->stun->GetSocketType(),
                                                        relayedCandidateStunActivity,
                                                        permissionStunActivity);

                    // store relayed candidate
                    stunActivity->stun->GetComponent()->AddCandidate(relayedCandidate);

                    // Set the relay IP and port in the STUN object
                    stunActivity->stun->SetTurnAddr(relayed.addr);
                    stunActivity->stun->SetTurnPort(relayed.port);
                    QCC_DbgPrintf(("Setting Relay address %s and port %d in STUN object", relayed.addr.ToString().c_str(), relayed.port));

                    sharedStunRelayedCandidate = relayedCandidate; // to demux received check messages later
                }
            }
            // cease retries
            if (NULL != retransmit) {
                retransmit->SetState(Retransmit::ReceivedSuccessResponse);
            }
            break;
        }

        case STUN_ATTR_LIFETIME: {
            const StunAttributeLifetime& sa = *reinterpret_cast<StunAttributeLifetime*>(*stunAttrIt);
            grantedAllocationLifetimeSecs = sa.GetLifetime();
            if (relayedCandidate) {
                relayedCandidate->SetAllocationLifetimeSeconds(grantedAllocationLifetimeSecs);
            }

            break;
        }

        case STUN_ATTR_PRIORITY: {
            const StunAttributePriority& sa = *reinterpret_cast<StunAttributePriority*>(*stunAttrIt);
            requestPriority = sa.GetPriority();
            break;
        }

        case STUN_ATTR_USE_CANDIDATE:
            useCandidateRequest = true;
            break;

        case STUN_ATTR_ICE_CONTROLLING: {
            //const StunAttributeIceControlling& sa = *reinterpret_cast<StunAttributeIceControlling*>(*stunAttrIt);
            ICEcontrollingRequest = true;
            //controlTieBreaker = sa.GetValue();
            break;
        }

        case STUN_ATTR_ICE_CONTROLLED: {
            //const StunAttributeIceControlled& sa = *reinterpret_cast<StunAttributeIceControlled*>(*stunAttrIt);
            ICEcontrollingRequest = false;
            //controlTieBreaker = sa.GetValue();
            break;
        }


        case STUN_ATTR_ERROR_CODE: {
            const StunAttributeErrorCode& sa = *reinterpret_cast<StunAttributeErrorCode*>(*stunAttrIt);
            StunErrorCodes error;
            String reason;
            sa.GetError(error, reason);

            if (NULL != retransmit) {
                retransmit->SetState(Retransmit::ReceivedErrorResponse);

                switch (error) {
                case STUN_ERR_CODE_UNAUTHORIZED:
                    // Handle this special case of STUN 'error' by retrying with credentials
                    retransmit->SetState(Retransmit::ReceivedAuthenticateResponse);
                    retransmit->SetErrorCode(ER_STUN_AUTH_CHALLENGE);
                    break;

                case STUN_ERR_CODE_INSUFFICIENT_CAPACITY:
                    retransmit->SetErrorCode(ER_ICE_ALLOCATE_REJECTED_NO_RESOURCES);
                    //todo notify app
                    break;

                case STUN_ERR_CODE_ALLOCATION_QUOTA_REACHED:
                    retransmit->SetErrorCode(ER_ICE_ALLOCATION_QUOTA_REACHED);
                    break;

                case STUN_ERR_CODE_ALLOCATION_MISMATCH:
                    retransmit->SetErrorCode(ER_ICE_ALLOCATION_MISMATCH);
                    break;

                case STUN_ERR_CODE_ROLE_CONFLICT:
                    // Handle this special case of STUN 'error' by retrying with reversed role
                    checkStatus = ICECandidatePair::CheckRoleConflict;
                    break;

                default:
                    status = ER_ICE_STUN_ERROR;
                    checkStatus = ICECandidatePair::CheckGenericFailed;
                    retransmit->SetErrorCode(status); //ToDo make these unique!!!
                    break;
                }
            }

            break;
        }

        case STUN_ATTR_USERNAME: {
            const StunAttributeUsername& sa = *reinterpret_cast<StunAttributeUsername*>(*stunAttrIt);
            sa.GetUsername(username);
            break;
        }

        default:
            break;
        }
    }

    switch (msg.GetTypeClass()) {
    case STUN_MSG_ERROR_CLASS:
        // ToDo report which errors?
        // component->GetICEStream()->GetSession()->UpdateICEStreamStates();
        break;

    case STUN_MSG_INDICATION_CLASS:
        // ignore
        QCC_DbgPrintf(("Unexpected STUN_MSG_INDICATION_CLASS"));
        break;

    case STUN_MSG_REQUEST_CLASS:
    {
        if (msg.GetTypeMethod() != STUN_MSG_BINDING_METHOD) {
            goto exit;
        }

        if (!component->GetICEStream()->GetSession()->ChecksStarted()) {
            // We haven't received peer's candidates yet via offer/answer.
            // Simply respond with no further state change.
            SendResponse(checkStatus, remote, receivedMsgWasRelayed, tid);
            goto exit;
        }

        if (component->GetICEStream()->GetSession()->GetRemoteInitiatedCheckUsername() != username) {
            // Username fragment does not match.

            // ToDo: the server MUST reject the request
            // with an error response.  This response MUST use an error code
            // of 401 (Unauthorized).

            goto exit;
        }

        // Section 7.2.1.1 draft-ietf-mmusic-ice-19
        checkStatus = ICECandidatePair::CheckResponseSent;  // assume everything is ok

        if (component->GetICEStream()->GetSession()->IsControllingAgent()) {
            if (ICEcontrollingRequest) {
/* ToDo: determine tieBreaker by matching intended pair (Need to know if this is a Relayed request.)
                if (constructedPair->GetControlTieBreaker() < controlTieBreaker) {
                    // switch to controlled role
                    component->GetICEStream()->GetSession()->SwapControllingAgent();
                }
                else {
                    checkStatus = ICECandidatePair::CheckRoleConflict;
                }
 */
            }
        } else {   // agent is in controlled role
            if (!ICEcontrollingRequest) { //ICE-CONTROLLED present in request
/* ToDo: determine tieBreaker by matching intended pair (Need to know if this is a Relayed request.)
                if ( constructedPair->GetControlTieBreaker() < controlTieBreaker) {
                    checkStatus = ICECandidatePair::CheckRoleConflict;
                }
                else {
                    // switch to controlled role
                    component->GetICEStream()->GetSession()->SwapControllingAgent();
                }
 */
            }
        }

        status = SendResponse(checkStatus, remote, receivedMsgWasRelayed, tid);

        if (ER_OK == status &&
            checkStatus == ICECandidatePair::CheckResponseSent) {

            // Section 7.2.1.3 draft-ietf-mmusic-ice-19
            String uniqueFoundation;
            ICECandidate* remoteCandidate = component->GetICEStream()->MatchRemoteCandidate(remote, uniqueFoundation);
            if (!remoteCandidate) {
                remoteCandidate = new ICECandidate(ICECandidate::PeerReflexive_Candidate,
                                                   remote, component, transportProtocol, requestPriority, uniqueFoundation);

                // Add remote peer-reflexive candidate to our list.
                component->GetICEStream()->AddRemoteCandidate(remoteCandidate);
            }

            // Section 7.2.1.4 draft-ietf-mmusic-ice-19
            // 'Construct' a pair, meaning find a pair whose local candidate is equal to
            // the transport address on which the STUN request was received, and a
            // remote candidate equal to the source transport address where the
            // request came from (which may be peer-reflexive remote candidate that was just learned).
            // Recall that this Stun object may be shared by multiple local
            // candidates (host, server-reflexive, relayed,) each belonging to perhaps multiple candidate pairs.
            ICECandidatePair* constructedPair = NULL;
            if (receivedMsgWasRelayed) {
                constructedPair = component->GetICEStream()->MatchCheckListEndpoint(sharedStunRelayedCandidate->endPoint, remote);
            } else {
                constructedPair = component->GetICEStream()->MatchCheckListEndpoint(endPoint, remote);
                if (NULL == constructedPair) {
                    constructedPair = component->GetICEStream()->MatchCheckListEndpoint(sharedStunServerReflexiveCandidate->endPoint, remote);
                }
            }

            if (!constructedPair) {
                // pair is not currently on check list
                uint64_t pairPriority =
                    component->GetICEStream()->GetSession()->ComputePairPriority(
                        component->GetICEStream()->GetSession()->IsControllingAgent(),
                        0,                         //ToDo constructedPair->local.GetPriority(),
                        remoteCandidate->GetPriority());

                constructedPair = new ICECandidatePair(*this, *remoteCandidate, false, pairPriority);
                // Insert into checkList based on priority.
                component->GetICEStream()->AddCandidatePairByPriority(constructedPair);

                constructedPair->AddTriggered();

                QCC_DbgPrintf(("Added new pair local %s:%d remote %s:%d",
                               constructedPair->local.GetEndpoint().addr.ToString().c_str(),
                               constructedPair->local.GetEndpoint().port,
                               constructedPair->remote.endPoint.addr.ToString().c_str(), constructedPair->remote.endPoint.port));
            } else {
                // Pair is on check list
                switch (constructedPair->state) {
                case ICECandidatePair::Waiting:
                    constructedPair->AddTriggered();
                    break;

                case ICECandidatePair::Frozen:
                    constructedPair->AddTriggered();   // ToDo: spec doesn't explicitly say to, but
                    // we change to Waiting here. Correct?
                    break;

                case ICECandidatePair::InProgress:
                    // Cancel the in-progress transaction and any remaining retransmits,
                    // and trigger a new check.
                    constructedPair->SetCanceled();
                    constructedPair->AddTriggered();
                    break;

                case ICECandidatePair::Failed:
                    constructedPair->AddTriggered();
                    break;

                case ICECandidatePair::Succeeded:
                    // do nothing
                    break;
                }
            }

            // Section 7.2.1.5 draft-ietf-mmusic-ice-19
            if (useCandidateRequest && !component->GetICEStream()->GetSession()->IsControllingAgent()) {
                if (constructedPair->state == ICECandidatePair::Succeeded) {
                    constructedPair->SetNominated();
                    QCC_DbgPrintf(("SetNominated (CONTROLLED) local %s:%d remote %s:%d",
                                   constructedPair->local.endPoint.addr.ToString().c_str(), constructedPair->local.endPoint.port,
                                   constructedPair->remote.endPoint.addr.ToString().c_str(), constructedPair->remote.endPoint.port));

                } else if (constructedPair->state == ICECandidatePair::InProgress ||
                           constructedPair->state == ICECandidatePair::Waiting) { //EqualsCanceledTransactionID??
                    constructedPair->SetNominatedContingent();
                    QCC_DbgPrintf(("SetNominatedContingent (CONTROLLED) local %s:%d remote %s:%d",
                                   constructedPair->local.endPoint.addr.ToString().c_str(), constructedPair->local.endPoint.port,
                                   constructedPair->remote.endPoint.addr.ToString().c_str(), constructedPair->remote.endPoint.port));
                }
            }

            component->GetICEStream()->GetSession()->UpdateICEStreamStates();
        }

        break;
    }

    case STUN_MSG_RESPONSE_CLASS:
    {
        // Is this an Allocate/CreatePermission/refresh response?
        if (NULL != retransmit) {
            if (retransmit->GetState() == Retransmit::AwaitingResponse) {
                retransmit->SetState(Retransmit::ReceivedSuccessResponse);
                QCC_DbgPrintf(("ReceivedSuccessResponse"));
            }
        } else {
            // Check if this is a response to our request.
            ICECandidatePair* intendedPair = component->GetICEStream()->MatchCheckList(remote, tid);
            if (intendedPair) {
                if ((ICECandidatePair::InProgress == intendedPair->state) ||
                    ((ICECandidatePair::Waiting == intendedPair->state) &&     // canceled?
                     intendedPair->EqualsCanceledTransactionID(tid))) {

                    QCC_DbgPrintf(("CheckSucceeded"));

                    // Notify the stream object
                    component->GetICEStream()->ProcessCheckEvent(intendedPair,
                                                                 ICECandidatePair::CheckSucceeded, mappedAddress);
                }

                component->GetICEStream()->GetSession()->UpdateICEStreamStates();
            }
        }
        break;
    }

    default:
        break;
    }


exit:
    component->GetICEStream()->GetSession()->Unlock();

    return status;
}

QStatus ICECandidate::SendResponse(uint16_t checkStatus, IPEndpoint& dest,
                                   bool usingTurn, StunTransactionID tid)
{
    QStatus status = ER_OK;
    StunMessage*msg = NULL;

    if (checkStatus == ICECandidatePair::CheckRoleConflict) {
        msg = new StunMessage(STUN_MSG_ERROR_CLASS,
                              STUN_MSG_BINDING_METHOD,
                              component->GetICEStream()->GetSession()->GetRemoteInitiatedCheckHmacKey(),
                              component->GetICEStream()->GetSession()->GetRemoteInitiatedCheckHmacKeyLength(), tid);
        msg->AddAttribute(new StunAttributeErrorCode(STUN_ERR_CODE_ROLE_CONFLICT, "Role Conflict"));
    } else {
        msg = new StunMessage(STUN_MSG_RESPONSE_CLASS,
                              STUN_MSG_BINDING_METHOD,
                              component->GetICEStream()->GetSession()->GetRemoteInitiatedCheckHmacKey(),
                              component->GetICEStream()->GetSession()->GetRemoteInitiatedCheckHmacKeyLength(), tid);
    }

    QCC_DbgPrintf(("Send Response: class %s, TID %s dest %s:%d",
                   msg->MessageClassToString(msg->GetTypeClass()).c_str(),
                   tid.ToString().c_str(),
                   dest.addr.ToString().c_str(), dest.port));

    msg->AddAttribute(new StunAttributeXorMappedAddress(*msg, dest.addr, dest.port));
    msg->AddAttribute(new StunAttributeRequestedTransport(REQUESTED_TRANSPORT_TYPE_UDP));
    msg->AddAttribute(new StunAttributeMessageIntegrity(*msg));
    msg->AddAttribute(new StunAttributeFingerprint(*msg));

    // send our response
    status = stunActivity->stun->SendStunMessage(*msg, dest.addr, dest.port, usingTurn);

    delete msg;

    return status;
}

String ICECandidate::GetPriorityString(void) const
{
    return U32ToString((uint32_t)priority, 10);
}

String ICECandidate::GetTypeString(void) const
{
    String s;

    switch (type) {
    case Host_Candidate:
        s = "host";
        break;

    case ServerReflexive_Candidate:
        s = "srflx";
        break;

    case Relayed_Candidate:
        s = "relay";
        break;

    case PeerReflexive_Candidate:
        s = "prflx";
        break;

    default:
        s = "unk";
        break;
    }

    return s;
}

} //namespace ajn
