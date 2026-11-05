#pragma once

#include "kdeworkspace.h"
#include "switchcontroller.h"
#include <qqmlregistration.h>

class PagerBackend : public SwitchController
{
    Q_OBJECT
    QML_ELEMENT
public:
    explicit PagerBackend(QObject *parent = nullptr)
        : SwitchController(new KdeWorkspace, parent)
    {
        workspace()->setParent(this);
    }
};
