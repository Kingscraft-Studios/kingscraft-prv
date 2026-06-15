////////////////////////////////////////////////////////////////////////////////////////////////////
// NoesisGUI - http://www.noesisengine.com
// Copyright (c) 2013 Noesis Technologies S.L. All Rights Reserved.
////////////////////////////////////////////////////////////////////////////////////////////////////


#include <NsApp/BackgroundEffectBehavior.h>
#include <NsGui/AdornerLayer.h>
#include <NsGui/Adorner.h>
#include <NsGui/Border.h>
#include <NsGui/Panel.h>
#include <NsGui/Shape.h>
#include <NsGui/Geometry.h>
#include <NsGui/VisualBrush.h>
#include <NsGui/Effect.h>
#include <NsGui/Binding.h>
#include <NsGui/BindingOperations.h>
#include <NsGui/ContentPropertyMetaData.h>
#include <NsGui/UIElementData.h>
#include <NsGui/PropertyMetadata.h>
#include <NsDrawing/Thickness.h>
#include <NsCore/ReflectionImplement.h>


using namespace NoesisApp;

////////////////////////////////////////////////////////////////////////////////////////////////////
namespace
{
struct EffectAdorner: public Noesis::Adorner
{
    EffectAdorner(Noesis::FrameworkElement* adornedElement, BackgroundEffectBehavior* behavior,
        Noesis::VisualBrush* targetBrush): Noesis::Adorner(adornedElement),
        mTargetBrush(targetBrush)
    {
        Noesis::Ptr<Noesis::VisualBrush> brush = *new Noesis::VisualBrush();
        Noesis::BindingOperations::SetBinding(brush, Noesis::VisualBrush::VisualProperty,
            Noesis::MakePtr<Noesis::Binding>("Source", behavior));
        brush->SetViewboxUnits(Noesis::BrushMappingMode_Absolute);

        Noesis::Ptr<Noesis::Border> border = *new Noesis::Border();
        Noesis::BindingOperations::SetBinding(border, Noesis::UIElement::EffectProperty,
            Noesis::MakePtr<Noesis::Binding>("Effect", behavior));
        border->SetBackground(brush);

        mChild = border;
        AddVisualChild(mChild);

        mTargetBrush->SetVisual(border);
        mTargetBrush->SetViewboxUnits(Noesis::BrushMappingMode_Absolute);

        SetOpacity(0.0f);
        SetIsHitTestVisible(false);
    }

    ~EffectAdorner()
    {
        RemoveVisualChild(mChild);
        mChild.Reset();
    }

    // From Visual
    //@{
    uint32_t GetVisualChildrenCount() const override
    {
        return 1;
    }

    Visual* GetVisualChild(uint32_t index) const override
    {
        NS_ASSERT(index == 0);
        return mChild;
    }
    //@}

    // From FrameworkElement
    //@{
    Noesis::Size MeasureOverride(const Noesis::Size&) override
    {
        Noesis::Point offset;
        Noesis::UIElement* adornedElement = GetAdornedElement();

        if (Noesis::DynamicCast<Noesis::Border*>(adornedElement) != nullptr)
        {
            Noesis::Border* border = (Noesis::Border*)adornedElement;
            const Noesis::Thickness& thickness = border->GetBorderThickness();
            mAdornedSize = adornedElement->GetRenderSize();
            mAdornedSize.width -= thickness.left + thickness.right;
            mAdornedSize.height -= thickness.top + thickness.bottom;
            offset.x = thickness.left;
            offset.y = thickness.top;
        }
        else if (Noesis::DynamicCast<Noesis::Panel*>(adornedElement) != nullptr)
        {
            mAdornedSize = adornedElement->GetRenderSize();
        }
        else if (Noesis::DynamicCast<Noesis::Shape*>(adornedElement) != nullptr)
        {
            struct PublicShape: public Noesis::Shape
            {
                Noesis::Geometry* GetRenderGeometryPublic() const { return GetRenderGeometry(); }
            };
            PublicShape* shape = (PublicShape*)adornedElement;
            shape->Measure(shape->GetMeasureConstraint());
            shape->Arrange(shape->GetArrangeConstraint());
            Noesis::Geometry* geo = shape->GetRenderGeometryPublic();
            Noesis::Rect bounds = geo->GetBounds();
            mAdornedSize = bounds.GetSize();
            offset = bounds.GetTopLeft();
        }

        Noesis::VisualBrush* sourceBrush = (Noesis::VisualBrush*)mChild->GetBackground();
        Noesis::Visual* source = sourceBrush->GetVisual();
        if (source != nullptr)
        {
            Noesis::Matrix4 mtx = adornedElement->TransformToVisual(source);
            offset += mtx[3].XY();
            mAdornedSize.width = mAdornedSize.width * mtx[0][0];
            mAdornedSize.height = mAdornedSize.height * mtx[1][1];

            sourceBrush->SetViewbox(Noesis::Rect(offset, mAdornedSize));
            mTargetBrush->SetViewbox(Noesis::Rect(mAdornedSize));
        }

        mChild->Measure(mAdornedSize);
        return mAdornedSize;
    }

    Noesis::Size ArrangeOverride(const Noesis::Size&) override
    {
        mChild->Arrange(Noesis::Rect(mAdornedSize));
        return mAdornedSize;
    }
    //@}

    // From Adorner
    //@{
    Noesis::Matrix4 GetDesiredTransform(const Noesis::Matrix4& transform) const override
    {
        Noesis::Matrix4 mtx = Noesis::Matrix4::Identity();
        mtx[3][0] = transform[3][0];
        mtx[3][1] = transform[3][1];
        return mtx;
    }
    //@}

private:
    Noesis::Ptr<Noesis::Border> mChild;
    Noesis::Ptr<Noesis::VisualBrush> mTargetBrush;
    Noesis::Size mAdornedSize;

    NS_IMPLEMENT_INLINE_REFLECTION_(EffectAdorner, Noesis::Adorner)
};
}

////////////////////////////////////////////////////////////////////////////////////////////////////
BackgroundEffectBehavior::BackgroundEffectBehavior()
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////
BackgroundEffectBehavior::~BackgroundEffectBehavior()
{
    Noesis::UIElement* source = GetSource();
    if (source != nullptr)
    {
        source->SubtreeDrawingCommandsChanged() -= Noesis::MakeDelegate(this,
            &BackgroundEffectBehavior::InvalidateAdorner);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
Noesis::UIElement* BackgroundEffectBehavior::GetSource() const
{
    return GetValue<Noesis::Ptr<Noesis::UIElement>>(SourceProperty);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void BackgroundEffectBehavior::SetSource(Noesis::UIElement* value)
{
    SetValue<Noesis::Ptr<Noesis::UIElement>>(SourceProperty, value);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
Noesis::Effect* BackgroundEffectBehavior::GetEffect() const
{
    return GetValue<Noesis::Ptr<Noesis::Effect>>(EffectProperty);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void BackgroundEffectBehavior::SetEffect(Noesis::Effect* value)
{
    SetValue<Noesis::Ptr<Noesis::Effect>>(EffectProperty, value);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
Noesis::Ptr<Noesis::Freezable> BackgroundEffectBehavior::CreateInstanceCore() const
{
    return *new BackgroundEffectBehavior();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void BackgroundEffectBehavior::OnAttached()
{
    Noesis::FrameworkElement* element = GetAssociatedObject();
    element->Loaded() += Noesis::MakeDelegate(this, &BackgroundEffectBehavior::OnLoaded);
    element->Unloaded() += Noesis::MakeDelegate(this, &BackgroundEffectBehavior::OnUnloaded);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void BackgroundEffectBehavior::OnDetaching()
{
    Noesis::FrameworkElement* element = GetAssociatedObject();
    element->Loaded() -= Noesis::MakeDelegate(this, &BackgroundEffectBehavior::OnLoaded);
    element->Unloaded() -= Noesis::MakeDelegate(this, &BackgroundEffectBehavior::OnUnloaded);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void BackgroundEffectBehavior::OnLoaded(Noesis::BaseComponent*, const Noesis::RoutedEventArgs&)
{
    Noesis::FrameworkElement* element = GetAssociatedObject();
    const Noesis::DependencyProperty* targetProperty = nullptr;
    if (Noesis::DynamicCast<Noesis::Border*>(element) != nullptr)
    {
        targetProperty = Noesis::Border::BackgroundProperty;
    }
    else if (Noesis::DynamicCast<Noesis::Panel*>(element) != nullptr)
    {
        targetProperty = Noesis::Panel::BackgroundProperty;
    }
    else if (Noesis::DynamicCast<Noesis::Shape*>(element) != nullptr)
    {
        targetProperty = Noesis::Shape::FillProperty;
    }

    if (targetProperty != nullptr)
    {
        Noesis::Ptr<Noesis::VisualBrush> brush = *new Noesis::VisualBrush();
        element->SetValue<Noesis::Ptr<Noesis::Brush>>(targetProperty, brush);

        mAdorner = *new EffectAdorner(element, this, brush);

        Noesis::AdornerLayer* adorners = Noesis::AdornerLayer::GetAdornerLayer(element);
        NS_ASSERT(adorners != nullptr);
        adorners->Add(mAdorner);

        element->DependencyPropertyChanged() += Noesis::MakeDelegate(this,
            &BackgroundEffectBehavior::OnAdornedElementChanged);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void BackgroundEffectBehavior::OnUnloaded(Noesis::BaseComponent*, const Noesis::RoutedEventArgs&)
{
    if (mAdorner != nullptr)
    {
        Noesis::FrameworkElement* element = GetAssociatedObject();
        Noesis::AdornerLayer* adorners = Noesis::AdornerLayer::GetAdornerLayer(element);
        NS_ASSERT(adorners != nullptr);
        adorners->Remove(mAdorner);

        mAdorner.Reset();

        element->DependencyPropertyChanged() -= Noesis::MakeDelegate(this,
            &BackgroundEffectBehavior::OnAdornedElementChanged);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void BackgroundEffectBehavior::OnAdornedElementChanged(Noesis::BaseComponent*,
    const Noesis::DependencyPropertyChangedEventArgs& e)
{
    if (e.prop == Noesis::Border::BorderThicknessProperty ||
        e.prop == Noesis::Shape::StrokeThicknessProperty ||
        e.prop == Noesis::Shape::StretchProperty)
    {
        InvalidateAdorner(nullptr);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void BackgroundEffectBehavior::InvalidateAdorner(Noesis::Visual*)
{
    if (mAdorner != nullptr)
    {
        Noesis::FrameworkElement* element = GetAssociatedObject();
        Noesis::AdornerLayer* adorners = Noesis::AdornerLayer::GetAdornerLayer(element);
        NS_ASSERT(adorners != nullptr);
        adorners->Update(element);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
NS_BEGIN_COLD_REGION

NS_IMPLEMENT_REFLECTION(BackgroundEffectBehavior, "NoesisGUIExtensions.BackgroundEffectBehavior")
{
    NsMeta<Noesis::ContentPropertyMetaData>("Effect");

    auto OnSourceChanged = [](Noesis::DependencyObject* d,
        const Noesis::DependencyPropertyChangedEventArgs& e)
    {
        BackgroundEffectBehavior* behavior = Noesis::DynamicCast<BackgroundEffectBehavior*>(d);
        if (behavior != nullptr)
        {
            Noesis::UIElement* oldSource = e.OldValue<Noesis::Ptr<Noesis::UIElement>>();
            if (oldSource != nullptr)
            {
                oldSource->SubtreeDrawingCommandsChanged() -= Noesis::MakeDelegate(behavior,
                    &BackgroundEffectBehavior::InvalidateAdorner);
            }
            Noesis::UIElement* newSource = e.NewValue<Noesis::Ptr<Noesis::UIElement>>();
            if (newSource != nullptr)
            {
                newSource->SubtreeDrawingCommandsChanged() += Noesis::MakeDelegate(behavior,
                    &BackgroundEffectBehavior::InvalidateAdorner);
            }
        }
    };

    Noesis::UIElementData* data = NsMeta<Noesis::UIElementData>(Noesis::TypeOf<SelfClass>());
    data->RegisterProperty<Noesis::Ptr<Noesis::UIElement>>(SourceProperty, "Source",
        Noesis::PropertyMetadata::Create(Noesis::Ptr<Noesis::UIElement>(),
            Noesis::PropertyChangedCallback(OnSourceChanged)));
    data->RegisterProperty<Noesis::Ptr<Noesis::Effect>>(EffectProperty, "Effect",
        Noesis::PropertyMetadata::Create(Noesis::Ptr<Noesis::Effect>()));
}

NS_END_COLD_REGION

////////////////////////////////////////////////////////////////////////////////////////////////////
const Noesis::DependencyProperty* BackgroundEffectBehavior::SourceProperty;
const Noesis::DependencyProperty* BackgroundEffectBehavior::EffectProperty;
