#include <mutex>
#include <thread>
#include <iostream>
#include "../deps/kazlog/kazlog.h"
#include "ui_private.h"
#include "interface.h"
#include "../utils/unicode.h"

namespace kglt {
namespace ui {

Element ElementImpl::append(const unicode& tag) {
    TiXmlElement* element = new TiXmlElement(tag.lstrip("<").rstrip(">").encode());
    element_->LinkEndChild(element);
    interface_->element_impls_[element] = std::make_shared<ElementImpl>(interface_, element);
    return Element(interface_->element_impls_[element]);
}

std::string ElementImpl::name() const {
    return element_->ValueStr();
}

float ElementImpl::left() const {

}

float ElementImpl::top() const {

}

float ElementImpl::width() const {

}

float ElementImpl::height() const {

}

void ElementImpl::set_event_callback(const unicode& event_type, std::function<bool (Event)> func) {
    event_callbacks_[event_type] = func;
}

void ElementImpl::set_text(const unicode& text) {
    element_->RemoveChild(element_->FirstChild());

    TiXmlText* text_node = new TiXmlText(text.encode());
    element_->LinkEndChild(text_node);
}

void ElementImpl::inner_rml(const unicode& rml) {

}


}
}
