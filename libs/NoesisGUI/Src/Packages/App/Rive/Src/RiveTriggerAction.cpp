////////////////////////////////////////////////////////////////////////////////////////////////////
// NoesisGUI - http://www.noesisengine.com
// Copyright (c) 2013 Noesis Technologies S.L. All Rights Reserved.
////////////////////////////////////////////////////////////////////////////////////////////////////


#include <NsApp/RiveTriggerAction.h>
#include <NsGui/UIElementData.h>
#include <NsGui/PropertyMetadata.h>


using namespace NoesisApp;


////////////////////////////////////////////////////////////////////////////////////////////////////
const char* RiveTriggerAction::GetTriggerPath() const
{
    return GetValue<Noesis::String>(TriggerPathProperty).Str();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void RiveTriggerAction::SetTriggerPath(const char* path)
{
    SetValue<Noesis::String>(TriggerPathProperty, path);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
const char* RiveTriggerAction::GetTriggerName() const
{
    return GetValue<Noesis::String>(TriggerNameProperty).Str();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void RiveTriggerAction::SetTriggerName(const char* name)
{
    SetValue<Noesis::String>(TriggerNameProperty, name);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
Noesis::Ptr<Noesis::Freezable> RiveTriggerAction::CreateInstanceCore() const
{
    return *new RiveTriggerAction();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void RiveTriggerAction::Invoke(Noesis::BaseComponent*)
{
    RiveControl* rive = GetTarget();
    if (rive != nullptr)
    {
        rive->FireInputTrigger(GetTriggerPath(), GetTriggerName());
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
NS_BEGIN_COLD_REGION

NS_IMPLEMENT_REFLECTION(RiveTriggerAction, "NoesisGUIExtensions.RiveTriggerAction")
{
    Noesis::UIElementData* data = NsMeta<Noesis::UIElementData>(Noesis::TypeOf<RiveTriggerAction>());
    data->RegisterProperty<Noesis::String>(TriggerPathProperty, "TriggerPath",
        Noesis::PropertyMetadata::Create(Noesis::String()));
    data->RegisterProperty<Noesis::String>(TriggerNameProperty, "TriggerName",
        Noesis::PropertyMetadata::Create(Noesis::String()));
}

NS_END_COLD_REGION

////////////////////////////////////////////////////////////////////////////////////////////////////
const Noesis::DependencyProperty* RiveTriggerAction::TriggerPathProperty;
const Noesis::DependencyProperty* RiveTriggerAction::TriggerNameProperty;
