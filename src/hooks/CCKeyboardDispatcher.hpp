#pragma once

#include <Geode/Geode.hpp>
#include <Geode/modify/CCKeyboardDispatcher.hpp>
#include "EditorUI.hpp"

using namespace geode::prelude;

class $modify(MyCCKeyboardDispatcher, CCKeyboardDispatcher) {

    bool dispatchKeyboardMSG(cocos2d::enumKeyCodes key, bool down, bool repeated) {
        auto ret = CCKeyboardDispatcher::dispatchKeyboardMSG(key, down, repeated);
        if (EditorUI* editorUI = EditorUI::get()) {
            MyEditorUI* myEditorUI = static_cast<MyEditorUI*>(editorUI);
            if (!myEditorUI->m_fields->m_objectSelectPopup) {
                if (getAltKeyPressed() && down && !repeated) {
                    if (key == cocos2d::enumKeyCodes::KEY_Enter) {
                        myEditorUI->onCreativeMenu(nullptr);
                    }
                }
            }
        }
        return ret;
    }
};
