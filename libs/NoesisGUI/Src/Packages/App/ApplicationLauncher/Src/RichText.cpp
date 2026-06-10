////////////////////////////////////////////////////////////////////////////////////////////////////
// NoesisGUI - http://www.noesisengine.com
// Copyright (c) 2013 Noesis Technologies S.L. All Rights Reserved.
////////////////////////////////////////////////////////////////////////////////////////////////////


#include <NsApp/RichText.h>
#include <NsCore/ReflectionImplement.h>
#include <NsCore/UTF8.h>
#include <NsGui/Binding.h>
#include <NsGui/UIElementData.h>
#include <NsGui/PropertyMetadata.h>
#include <NsGui/TextBlock.h>
#include <NsGui/BitmapImage.h>
#include <NsGui/Bold.h>
#include <NsGui/Image.h>
#include <NsGui/InlineUIContainer.h>
#include <NsGui/Italic.h>
#include <NsGui/Run.h>
#include <NsGui/Span.h>
#include <NsGui/UICollection.h>
#include <NsGui/Underline.h>
#include <NsGui/Uri.h>
#include <NsGui/Style.h>
#include <NsGui/FontFamily.h>
#include <NsGui/Hyperlink.h>
#include <NsGui/LineBreak.h>
#include <NsGui/SolidColorBrush.h>


using namespace Noesis;
using namespace NoesisApp;

using Parameter = Pair<FixedString<8>, FixedString<128>>;


////////////////////////////////////////////////////////////////////////////////////////////////////
static const char* TryParse(const char* begin, const char* end, const TextBlock* textBlock,
    InlineCollection* inlines);

////////////////////////////////////////////////////////////////////////////////////////////////////
/// This method allows for the creation of Spans for BBCode tags which act as containers for other
/// Inlines.
///
/// Extend this method to add new container BBCode tags, creating a Span (or Span derived type)
/// for each.
///
/// If a Span has been created, return a pointer to that Span's InlineCollection. If no Span has
/// been created, return nullptr.
////////////////////////////////////////////////////////////////////////////////////////////////////
static InlineCollection* TryCreateSpanForTag(const char* tagName, ArrayRef<Parameter> parameters,
    const TextBlock* textBlock, InlineCollection* inlineCollection)
{
    if (StrCaseEquals(tagName, "b"))
    {
        const Ptr<Bold> bold = MakePtr<Bold>();
        inlineCollection->Add(bold);
        return bold->GetInlines();
    }
    if (StrCaseEquals(tagName, "i"))
    {
        const Ptr<Italic> italic = MakePtr<Italic>();
        inlineCollection->Add(italic);
        return italic->GetInlines();
    }
    if (StrCaseEquals(tagName, "u"))
    {
        const Ptr<Underline> underline = MakePtr<Underline>();
        inlineCollection->Add(underline);
        return underline->GetInlines();
    }
    if (StrCaseEquals(tagName, "url"))
    {
        for (const Parameter& element : parameters)
        {
            if (element.first == "url") 
            {
                // Only create Hyperlink as a container if the url is specified as a parameter
                const Ptr<Hyperlink> hyperlink = MakePtr<Hyperlink>();
                hyperlink->SetNavigateUri(element.second.Str());
                inlineCollection->Add(hyperlink);
                return hyperlink->GetInlines();
            }
        }

        return nullptr;
    }
    if (StrCaseEquals(tagName, "font") || StrCaseEquals(tagName, "size")
        || StrCaseEquals(tagName, "color"))
    {
        const Ptr<Span> span = MakePtr<Span>();
        for (const Parameter& element : parameters)
        {
            if (element.first == "size")
            {
                uint32_t charsParsed;
                span->SetFontSize(StrToFloat(element.second.Str(), &charsParsed));
                if (charsParsed == 0)
                {
                    NS_ERROR("BBCode tag '%s' size property value '%s' is not a valid float",
                        element.first.Str(), element.second.Str());
                }
            }
            else if (element.first == "font")
            {
                Ptr<FontFamily> fontFamily = MakePtr<FontFamily>(element.second.Str());
                span->SetFontFamily(fontFamily);
            }
            else if (element.first == "color")
            {
                Ptr<SolidColorBrush> brush;
                if (SolidColorBrush::TryParse(element.second.Str(), brush))
                {
                    span->SetForeground(brush);
                }
                else
                {
                    NS_ERROR("BBCode tag '%s' color property value '%s' is not a valid color string",
                        element.first.Str(), element.second.Str());
                }
            }
        }
        inlineCollection->Add(span);
        return span->GetInlines();
    }
    if (StrCaseEquals(tagName, "style"))
    {
        const Ptr<Span> span = MakePtr<Span>();
        for (const Parameter& element : parameters)
        {
            if (element.first == "style")
            {
                Style* style = textBlock->FindResource<Style>(element.second.Str());
                if (style != nullptr)
                {
                    span->SetStyle(style);
                }
                else
                {
                    NS_ERROR("BBCode style tag parameter value '%s' not found.",
                        element.second.Str());
                }
            }
        }
        inlineCollection->Add(span);
        return span->GetInlines();
    }
    
    return nullptr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// This method allows for the creation of Inlines for BBCode tags which do not act as containers
/// for other Inlines.
/// 
/// Extend this method to add new non-container BBCode tags, creating an Inline derived type for
/// each.
////////////////////////////////////////////////////////////////////////////////////////////////////
static void TryCreateInlineForTag(const char* tagName, const char* content,
    ArrayRef<Parameter> parameters, InlineCollection* inlineCollection)
{
    if (StrCaseEquals(tagName, "br"))
    {
        const Ptr<LineBreak> lineBreak = MakePtr<LineBreak>();
        inlineCollection->Add(lineBreak);
        return;
    }
    if (StrCaseEquals(tagName, "url"))
    {
        const Ptr<Hyperlink> hyperlink = MakePtr<Hyperlink>(MakePtr<Run>(content));
        hyperlink->SetNavigateUri(content);
        inlineCollection->Add(hyperlink);
        return;
    }
    if (StrCaseEquals(tagName, "bind"))
    {
        const Ptr<Run> run = MakePtr<Run>();
        const Ptr<Binding> binding = MakePtr<Binding>(content);

        for (const Parameter& element : parameters)
        {
            if (element.first == "format")
            {
                binding->SetStringFormat(element.second.Str());
            }
        }

        run->SetBinding(Run::TextProperty, binding);
        inlineCollection->Add(run);
        return;
    }
    if (StrCaseEquals(tagName, "img"))
    {
        Ptr<Noesis::Image> image = MakePtr<Noesis::Image>();
        image->SetSource(MakePtr<BitmapImage>(Uri(content)));
        inlineCollection->Add(MakePtr<InlineUIContainer>(image));
        for (const Parameter& element : parameters)
        {
            if (element.first == "width")
            {
                uint32_t charsParsed;
                image->SetWidth(StrToFloat(element.second.Str(), &charsParsed));
                if (charsParsed == 0)
                {
                    NS_ERROR("Invalid value for BBCode img tag width parameter");
                }
            }
            else if (element.first == "height")
            {
                uint32_t charsParsed;
                image->SetHeight(StrToFloat(element.second.Str(), &charsParsed));
                if (charsParsed == 0)
                {
                    NS_ERROR("Invalid value for BBCode img tag height parameter");
                }
            }
        }
        if (parameters.Size() > 1)
        {
            image->SetStretch(Stretch_Fill);
        }
        return;
    }
    
    NS_ERROR("BBCode tag '%s' is not currently supported", tagName);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
static bool IsAlphaNumeric(uint32_t c)
{
    return (c >= '0' && c <= '9') || (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
}

////////////////////////////////////////////////////////////////////////////////////////////////////
static const char* ParseText(const char* begin, const char* end, InlineCollection* inlines)
{
    NS_ASSERT(begin != end);
    NS_ASSERT(*begin != '[');
    const char* current = begin;
    FixedString<256> text;

    for (; current != end; current++)
    {
        if (*current == '\\')
        {
            text.Append(begin, current);
            ++current;
            begin = current;

            // Safe exit if \ is the last char
            if (current == end)
            {
                break;
            }
        }
        else if (*current == ']')
        {
            NS_ERROR("BBCode contains a malformed closing bracket");
            return end;
        }
        else if (*current == '[')
        {
            break;
        }
    }

    if (current == begin)
    {
        current = end;
    }

    text.Append(begin, current);
    inlines->Add(MakePtr<Run>(text.Str()));

    return current;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
static const char* ParseContent(const char* begin, const char* end, BaseString& buffer)
{
    NS_ASSERT(begin <= end);
    const char* current = begin;

    bool useQuotationMarks = false;
    bool singleQuotes = false;

    if (*current == '\'' || *current == '"')
    {
        useQuotationMarks = true;
        singleQuotes = *current == '\'';
        ++begin;
        ++current;
    }

    while (current != end)
    {
        if (*current == '\\')
        {
            buffer.Append(begin, current);
            ++current;
            begin = current;
        }
        else if(useQuotationMarks)
        {
            if ((singleQuotes && *current == '\'') || (!singleQuotes && *current == '"'))
            {
                buffer.Append(begin, current);
                return current + 1;
            }
            if (*current == '[' || *current == ']')
            {
                NS_ERROR("BBCode parameter value is missing a closing quotation mark");
                return end;
            }
            ++current;
        }
        else
        {
            if (*current == '[' || *current == ']')
            {
                break;
            }
            ++current;
        }
    }

    buffer.Append(begin, current);
    return current;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
static const char* ParseName(const char* begin, const char* end, BaseString& buffer)
{
    NS_ASSERT(begin <= end);
    const char* utf8Current = begin;
    const char* utf8Next = begin;
    uint32_t c = UTF8::Next(utf8Next);

    for (const char* it = begin; it != end; it++)
    {
        if (it == utf8Current)
        {
            if (!IsAlphaNumeric(c))
            {
                buffer.Assign(begin, it);
                return it;
            }
            utf8Current = utf8Next;
            c = UTF8::Next(utf8Next);
        }
    }

    return begin;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
static const char* ParseKeyValuePair(const char* begin, const char* end,
    BaseVector<Parameter>& parameters)
{
    NS_ASSERT(begin < end);
    NS_ASSERT(*begin == ' ');

    const char* current = begin;
    while (*current == ' ')
    {
        ++current;
    }

    if (!IsAlphaNumeric(UTF8::Get(current)))
    {
        return current;
    }

    FixedString<8> key;
    current = ParseName(current, end, key);

    if (key.Empty())
    {
        NS_ERROR("A parameter for BBCode tag key is malformed");
        return end;
    }

    if (*current != '=')
    {
        NS_ERROR("A parameter for BBCode tag key '%s' is malformed", key.Str());
        return end;
    }

    FixedString<128> value;
    current = ParseContent(current + 1, end, value);

    if (value.Empty())
    {
        NS_ERROR("A parameter for BBCode tag key '%s' is malformed", key.Str());
        return end;
    }

    parameters.PushBack({ key, value });
    return current;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
static const char* ParseTag(const char* begin, const char* end, const TextBlock* textBlock,
    InlineCollection* inlines)
{
    NS_ASSERT(begin != end);
    NS_ASSERT(*begin == '[');

    // We start parsing after the '['
    const char* current = begin + 1;

    // 1) Parse tag name
    FixedString<8> tagName;
    current = ParseName(current, end, tagName);

    if (tagName.Empty())
    {
        NS_ERROR("A BBCode tag is malformed (contains no name)");
        return current;
    }

    Vector<Parameter, 2> parameters;

    // 2) Either parse '=' value or key/value pairs separated by spaces
    if (*current == '=')
    {
        FixedString<128> value;
        current = ParseContent(current + 1, end, value);
        parameters.PushBack({ tagName, value });
    }
    else
    {
        while (*current == ' ')
        {
            current = ParseKeyValuePair(current, end, parameters);
        }
    }

    // 3) Self-closing tag detection: "/]"
    if (*current == '/' && *(current + 1) == ']')
    {
        TryCreateInlineForTag(tagName.Str(), "", parameters, inlines);
        return current + 2;
    }

    // 4) Expect a closing ']' (end of opening tag)
    if (*current != ']')
    {
        NS_ERROR("BBCode tag '%s' has a malformed closing tag", tagName.Str());
        return end;
    }

    // 5) Parse the tag content
    InlineCollection* newInlines = TryCreateSpanForTag(tagName.Str(), parameters, textBlock, inlines);
    if (newInlines != nullptr)
    {
        inlines = newInlines;
        current = TryParse(current + 1, end, textBlock, inlines);
    }
    else
    {
        String content;
        current = ParseContent(current + 1, end, content);

        if (current != end)
        {
            current++;
        }

        TryCreateInlineForTag(tagName.Str(), content.Str(), parameters, inlines);
    }

    // 6) Expect closing '/' for end tag
    if (*current != '/')
    {
        NS_ERROR("BBCode tag '%s' is missing a closing tag", tagName.Str());
        return end;
    }

    current++;

    FixedString<8> closingTagName;
    current = ParseName(current, end, closingTagName);

    if (closingTagName != tagName)
    {
        NS_ERROR("BBCode tag opening name '%s' does not match closing tag name '%s'", tagName.Str(),
            closingTagName.Str());
        return end;
    }

    if (*current != ']')
    {
        NS_ERROR("BBCode tag '%s' has a malformed closing tag", tagName.Str());
        return end;
    }

    return current + 1;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
static const char* TryParse(const char* begin, const char* end, const TextBlock* textBlock,
    InlineCollection* inlines)
{
    NS_ASSERT(begin <= end);
    const char* current = begin;

    while (current != end)
    {
        if (*current == '[')
        {
            const char* next = current + 1;
            if (*next == '/')
            {
                return next;
            }
            current = ParseTag(current, end, textBlock, inlines);
        }
        else
        {
            current = ParseText(current, end, inlines);
        }
    }
    return current;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void RichText::SetText(DependencyObject* element, const char* value)
{
    NS_CHECK(element != nullptr, "Element is null");
    element->SetValue<String>(TextProperty, value);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
const char* RichText::GetText(const DependencyObject* element)
{
    NS_CHECK(element != nullptr, "Element is null");
    return element->GetValue<String>(TextProperty).Str();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
static void OnSourceChanged(DependencyObject* obj, const DependencyPropertyChangedEventArgs& args)
{
    const TextBlock* textBlock = DynamicCast<TextBlock*>(obj);
    if (textBlock)
    {
        InlineCollection* inlines = textBlock->GetInlines();
        inlines->Clear();

        const String& text = args.NewValue<String>();
        TryParse(text.Begin(), text.End(), textBlock, inlines);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
NS_BEGIN_COLD_REGION

NS_IMPLEMENT_REFLECTION(RichText, "NoesisGUIExtensions.RichText")
{
    UIElementData* data = NsMeta<UIElementData>(TypeOf<SelfClass>());
    data->RegisterProperty<String>(TextProperty, "Text", PropertyMetadata::Create(
        String(), PropertyChangedCallback(OnSourceChanged)));
}

NS_END_COLD_REGION

////////////////////////////////////////////////////////////////////////////////////////////////////
const DependencyProperty* RichText::TextProperty;