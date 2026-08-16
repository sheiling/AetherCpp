#pragma once

#include "service/flow/scan_flow_progress_bridge.h"
#include "service/flow/scan_flow_context.h"
#include "inspection_session.h"

#include <scheduler/context/flow_context.h>

namespace aoi::inspection {

inline std::shared_ptr<InspectionSession> sessionFrom(scheduler::FlowContext &ctx)
{
    return ctx.get<std::shared_ptr<InspectionSession>>(flow::kCtxSession);
}

inline std::shared_ptr<InspectionSession> sessionFrom(const scheduler::FlowContext &ctx)
{
    const auto opt = ctx.tryGet<std::shared_ptr<InspectionSession>>(flow::kCtxSession);
    return opt ? *opt : nullptr;
}

inline void reportSessionProgress(const std::shared_ptr<InspectionSession> &session)
{
    if (!session || !session->progressBridge)
        return;
    session->progressBridge->reportProgress(session->progressMessage,
                                            session->currentImageIndex,
                                            session->totalImageCount,
                                            session->lastFovPreview);
}

inline void reportSessionLog(const std::shared_ptr<InspectionSession> &session,
                             const QString &message,
                             const QString &level = QStringLiteral("INFO"))
{
    if (!session || !session->progressBridge)
        return;
    session->progressBridge->reportLog(message, level);
}

} // namespace aoi::inspection
