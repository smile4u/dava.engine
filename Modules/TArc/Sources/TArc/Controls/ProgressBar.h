#pragma once

#include "TArc/Controls/ControlProxy.h"
#include "TArc/Controls/CommonStrings.h"
#include "TArc/Utils/QtConnections.h"
#include "TArc/Qt/QtString.h"

#include <QProgressBar>

namespace DAVA
{
namespace TArc
{
class ProgressBar : public ControlProxyImpl<QProgressBar>
{
public:
    enum Fields : uint32
    {
        Value,
        Range,
        Format,
        FieldCount
    };

    DECLARE_CONTROL_PARAMS(Fields);
    ProgressBar(const Params& params, DataWrappersProcessor* wrappersProcessor, Reflection model, QWidget* parent = nullptr);
    ProgressBar(const Params& params, ContextAccessor* accessor, Reflection model, QWidget* parent = nullptr);

protected:
    void UpdateControl(const ControlDescriptor& changedFields) override;
};

} // namespace TArc
} // namespace DAVA
