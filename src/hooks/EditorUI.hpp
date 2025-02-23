#pragma once

#include <Geode/Geode.hpp>
#include <Geode/modify/EditorUI.hpp>
#include "EditButtonBar.hpp"
#include "../popups/ObjectSelectPopup.hpp"
#include "../ObjectNames.hpp"
#include "CCMenuItemSpriteExtra.hpp"
#include "../BlurNode.hpp"

using namespace geode::prelude;
using namespace std::placeholders;

struct GroupInfo : CCObject {};

class $modify(MyEditorUI, EditorUI) {

	static void onModify(auto& self) {
        (void) self.setHookPriorityAfterPost("EditorUI::init", "hjfod.betteredit");
        (void) self.setHookPriorityAfterPost("EditorUI::init", "alphalaneous.vanilla_pages");
    }

	struct Fields {
		std::map<int, Ref<GameObject>> m_gameObjects{};
		std::unordered_map<int, std::vector<std::pair<int, Ref<GameObject>>>> m_tabObjects{};
		CCMenu* m_creativeMenu;
		ObjectSelectPopup* m_objectSelectPopup = nullptr;
		CCNode* m_tooltip;
		CCNode* m_gradientBG;
		CCLayerGradient* m_newGradientBG;
		CCLayerColor* m_darkenedBG;
		Ref<CCLayerColor> m_cheatBG;
		CCNode* m_lineNode;
		CCLayerColor* m_linePart1;
		CCLayerColor* m_linePart2;
		CCScale9Sprite* m_tooltipBG;
		CCLabelBMFont* m_tooltipText;
		CCLabelBMFont* m_tooltipObjID;
		CCSprite* m_blurSprite;
		CCNode* m_mainNode;
		BlurNode* m_blur;
    	bool m_queueVisible = false;
	};

    bool init(LevelEditorLayer* editorLayer) {
		if (!EditorUI::init(editorLayer)) return false;
		auto fields = m_fields.self();
		ObjectNames::get();
    	Mod* mod = Mod::get();

		CCSprite* creativeSpr = CCSprite::create("search-btn.png"_spr);
		creativeSpr->setScale(0.5f);
		CCSprite* circleSpr = CCSprite::create("circle.png"_spr);
		circleSpr->setScale(0.5f);
		circleSpr->setAnchorPoint({0, 0});
		circleSpr->setZOrder(-1);
		circleSpr->setOpacity(10);
		circleSpr->setID("highlight");

		CCMenuItemSpriteExtra* creativeBtn = CCMenuItemSpriteExtra::create(creativeSpr, this, menu_selector(MyEditorUI::onCreativeMenu));
		static_cast<HoverEnabledCCMenuItemSpriteExtra*>(creativeBtn)->enableHover(std::bind(&MyEditorUI::onSearchHover, this, _1, _2, _3, _4));
		creativeBtn->setPosition({15, 15});
		creativeBtn->setID("creative-button"_spr);

		creativeBtn->addChild(circleSpr);

		CCSize winSize = CCDirector::get()->getWinSize();

		if (mod->getSettingValue<bool>("enable-new-tab-ui")) {
			fields->m_gradientBG = geode::cocos::getChildBySpriteName(this, "edit_barBG_001.png");
			fields->m_gradientBG->setVisible(false);
		}

		fields->m_newGradientBG = CCLayerGradient::create(mod->getSettingValue<ccColor4B>("gradient-top-color"), mod->getSettingValue<ccColor4B>("gradient-bottom-color"));
		fields->m_newGradientBG->setContentSize({winSize.width, m_toolbarHeight});
		fields->m_newGradientBG->setAnchorPoint({0, 0});
		fields->m_newGradientBG->setBlendFunc({GL_SRC_ALPHA, GL_ONE});
		fields->m_newGradientBG->setZOrder(-1);

		fields->m_darkenedBG = CCLayerColor::create({0, 0, 0, static_cast<GLubyte>(255 * mod->getSettingValue<float>("gradient-overlay-opacity"))});
		fields->m_darkenedBG->setContentSize({winSize.width, m_toolbarHeight});
		fields->m_darkenedBG->setAnchorPoint({0, 0});

		fields->m_lineNode = CCNode::create();
		ccColor3B lineColor = mod->getSettingValue<ccColor3B>("line-color");

		fields->m_linePart1 = CCLayerColor::create({255, 255, 255, 255});
		fields->m_linePart1->setContentSize({winSize.width, m_tabsMenu->getScale()});
		fields->m_linePart1->setAnchorPoint({0, 0});
		fields->m_linePart1->ignoreAnchorPointForPosition(false);
		fields->m_linePart1->setColor(lineColor);

		fields->m_linePart2 = CCLayerColor::create({255, 255, 255, 255});
		fields->m_linePart2->setContentSize({winSize.width, m_tabsMenu->getScale()});
		fields->m_linePart2->setAnchorPoint({1, 0});
		fields->m_linePart2->setPosition({winSize.width, 0});
		fields->m_linePart2->ignoreAnchorPointForPosition(false);
		fields->m_linePart2->setColor(lineColor);

		fields->m_lineNode->addChild(fields->m_linePart1);
		fields->m_lineNode->addChild(fields->m_linePart2);
		fields->m_lineNode->setAnchorPoint({0, 0});
		fields->m_lineNode->setPosition({0, m_toolbarHeight});
		fields->m_lineNode->setZOrder(9);

		std::pair<float, float> edges = getSelectedButtonEdges(this);

		fields->m_linePart1->setContentWidth(edges.first);
		fields->m_linePart2->setContentWidth(winSize.width - edges.second);

		if (mod->getSettingValue<bool>("enable-new-tab-ui")) {
			addChild(fields->m_newGradientBG);
			addChild(fields->m_darkenedBG);
			addChild(fields->m_lineNode);
		}

		fields->m_creativeMenu = CCMenu::create();
		fields->m_creativeMenu->setContentSize({30, 30});
		fields->m_creativeMenu->setAnchorPoint({1, 1});
		fields->m_creativeMenu->ignoreAnchorPointForPosition(false);
		fields->m_creativeMenu->setZOrder(9);
		fields->m_creativeMenu->setID("creative-menu"_spr);

		fields->m_tooltip = CCNode::create();
		fields->m_tooltip->setID("tooltip");
		fields->m_tooltip->setZOrder(110);
		fields->m_tooltipBG = CCScale9Sprite::create("square02_001.png");
		fields->m_tooltipBG->setOpacity(225);
		fields->m_tooltipBG->setAnchorPoint({0, 0});
		fields->m_tooltipBG->setZOrder(-11);
		fields->m_tooltipBG->setID("background");
		fields->m_tooltip->setVisible(false);
		fields->m_tooltip->setAnchorPoint({0, 0});
		fields->m_tooltipText = CCLabelBMFont::create("", "chatFont.fnt");
		fields->m_tooltipText->setAnchorPoint({0.f, 1.f});
		fields->m_tooltipText->setID("tooltip-label");
		fields->m_tooltipText->setScale(0.75);
		fields->m_tooltipText->setPositionX(5);
		fields->m_tooltipObjID = CCLabelBMFont::create("", "chatFont.fnt");
		fields->m_tooltipObjID->setAnchorPoint({0.f, 0.f});
		fields->m_tooltipObjID->setID("object-id-label");
		fields->m_tooltipObjID->setColor({0, 255, 75});
		fields->m_tooltipObjID->setScale(0.5);
		fields->m_tooltipObjID->setPositionX(5);

		fields->m_tooltip->addChild(fields->m_tooltipText);
		fields->m_tooltip->addChild(fields->m_tooltipBG);
		fields->m_tooltip->addChild(fields->m_tooltipObjID);
		fields->m_tooltip->setScale(m_tabsMenu->getScale() * mod->getSettingValue<float>("tooltip-scale"));

		addChild(fields->m_tooltip);

		float x = 85;
		float y = 90;
		
		if (CCNode* toolbarTogglesMenu = getChildByID("toolbar-toggles-menu")) {
			if (!Loader::get()->isModLoaded("hjfod.betteredit")) {
				toolbarTogglesMenu->setAnchorPoint({1, 0});
				toolbarTogglesMenu->setPosition({winSize.width - 3, 0});
			}
			x = toolbarTogglesMenu->getPositionX() - toolbarTogglesMenu->getScaledContentWidth() - 1;
			y = toolbarTogglesMenu->getPositionY() + toolbarTogglesMenu->getScaledContentHeight();
		}
		
		fields->m_creativeMenu->setPosition({x, y});
		fields->m_creativeMenu->setScale(m_tabsMenu->getScale());

		fields->m_creativeMenu->addChild(creativeBtn);
		addChild(fields->m_creativeMenu);

		m_tabsMenu->setPositionY(m_toolbarHeight);
		m_tabsMenu->setZOrder(5);
		m_tabsMenu->setAnchorPoint({0.5f, 0.f});

		if (CCNode* editTabsMenu = getChildByID("edit-tabs-menu")) {
			editTabsMenu->setPositionY(m_toolbarHeight);
			editTabsMenu->setZOrder(5);
			editTabsMenu->setAnchorPoint({0.5f, 0.f});
		}

		if (CCNode* deleteTabsMenu = getChildByID("delete-tabs-menu")) {
			deleteTabsMenu->setPositionY(m_toolbarHeight);
			deleteTabsMenu->setZOrder(5);
			deleteTabsMenu->setAnchorPoint({0.5f, 0.f});
		}

		if (mod->getSettingValue<bool>("enable-new-tab-ui")) {

			if (CCNode* buildTabNav = getChildByID("build-tabs-menu-navigation-menu")) {
				CCMenuItemSpriteExtra* prevButton = static_cast<CCMenuItemSpriteExtra*>(buildTabNav->getChildByID("prev-button"));
				CCMenuItemSpriteExtra* nextButton = static_cast<CCMenuItemSpriteExtra*>(buildTabNav->getChildByID("next-button"));
				SEL_MenuHandler prevSelectorCopy = prevButton->m_pfnSelector;

				prevButton->setUserObject("orig-selector"_spr, CallFuncExt::create([prevButton, prevSelectorCopy] {
					(prevButton->m_pListener->*prevSelectorCopy)(prevButton);
				}));
				prevButton->m_pfnSelector = menu_selector(MyEditorUI::arrowPrevHijack);
				SEL_MenuHandler nextSelectorCopy = nextButton->m_pfnSelector;

				nextButton->setUserObject("orig-selector"_spr, CallFuncExt::create([nextButton, nextSelectorCopy] {
					(nextButton->m_pListener->*nextSelectorCopy)(nextButton);
				}));
				nextButton->m_pfnSelector = menu_selector(MyEditorUI::arrowNextHijack);

				//hacky reset
				(nextButton->m_pListener->*nextSelectorCopy)(nextButton);
				(prevButton->m_pListener->*prevSelectorCopy)(prevButton);
			}


			for (CCNode* tab : CCArrayExt<CCNode*>(m_tabsMenu->getChildren())) {
				changeBG(tab, 127, 172, 86);
				HoverEnabledCCMenuItemSpriteExtra* hoverBtn = static_cast<HoverEnabledCCMenuItemSpriteExtra*>(tab);
				hoverBtn->setUserObject("orig-y"_spr, CCFloat::create(hoverBtn->getPositionY()));
				hoverBtn->enableHover(std::bind(&MyEditorUI::onTabHover, this, _1, _2, _3, _4));
			}
			changeBG(m_tabsMenu->getChildByTag(m_selectedTab), mod->getSettingValue<ccColor4B>("gradient-top-color").a, 255, 255 * mod->getSettingValue<float>("gradient-overlay-opacity"));
		}

		for (MyEditButtonBar* buttonBar : CCArrayExt<MyEditButtonBar*>(m_createButtonBars)) {
			int tab = buttonBar->m_tabIndex;
			if (tab <= -1 || tab >= 13) continue;
			if (!fields->m_tabObjects.contains(tab)) {
				for (int id : buttonBar->m_fields->m_objectIDs) {
					fields->m_tabObjects[tab].push_back({id, createGameObject(id, fields)});
				}
			}
		}
		#ifdef GEODE_IS_WINDOWS
		queueInMainThread([this, fields, winSize, mod] {

			int blurStrength = mod->getSettingValue<int>("blur-strength");

			if (blurStrength > 0) {
				if (CCNode* mainNode = m_editorLayer->getChildByID("main-node")) {
					fields->m_cheatBG = CCLayerColor::create(ccColor4B{m_editorLayer->m_background->getColor().r, m_editorLayer->m_background->getColor().g, m_editorLayer->m_background->getColor().b, 255});
					fields->m_cheatBG->setContentSize(winSize);
					fields->m_cheatBG->setZOrder(-100);
					fields->m_blur = BlurNode::create(mainNode, blurStrength);
					fields->m_blur->addNodeToVisit(fields->m_cheatBG);
					fields->m_blur->addNodeToIgnore(m_editorLayer->m_background);
					fields->m_blur->addNodeToIgnore(m_editorLayer->m_drawGridLayer);
					schedule(schedule_selector(MyEditorUI::checkBGColor));
					addChild(fields->m_blur->getBlurredSprite());
					fields->m_blur->getBlurredSprite()->setID("blur-sprite"_spr);
					fields->m_blur->setCrop({0, 0, winSize.width, m_toolbarHeight});
					addChild(fields->m_blur);
				}
			}
		});
		#endif
		return true;
	}

	void checkBGColor(float dt) {
		auto fields = m_fields.self();
		fields->m_blur->addNodeToIgnore(m_editorLayer->m_background);
		if (m_editorLayer->m_showGround) {
			fields->m_cheatBG->setPositionY(m_editorLayer->m_groundLayer->getPositionY());
		}
		else {
			fields->m_cheatBG->setPositionY(0);
		}
		fields->m_cheatBG->setColor(m_editorLayer->m_background->getColor());
	}

	void arrowPrevHijack(CCObject* sender) {
		static_cast<CCActionInstant*>(static_cast<CCNode*>(sender)->getUserObject("orig-selector"_spr))->update(0);
		setLines(EditorUI::get());
	}

	void arrowNextHijack(CCObject* sender) {
		static_cast<CCActionInstant*>(static_cast<CCNode*>(sender)->getUserObject("orig-selector"_spr))->update(0);
		setLines(EditorUI::get());
	}

	std::pair<float, float> getSelectedButtonEdges(EditorUI* editorUI) {

		if (!editorUI) return {0, 0};

		if (editorUI->m_selectedMode == 2) {
			CCNode* tab = editorUI->m_tabsMenu->getChildByTag(editorUI->m_selectedTab);
			if (!tab->isVisible()) return {0, 0};

			float xLeft = editorUI->m_tabsMenu->getPositionX() - editorUI->m_tabsMenu->getScaledContentWidth()/2;
			float tabLeft = tab->getPositionX() - tab->getScaledContentWidth()/2;

			float start = xLeft + tabLeft * editorUI->m_tabsMenu->getScale();
			float end = xLeft + (tabLeft + tab->getScaledContentWidth()) * editorUI->m_tabsMenu->getScale();

			return {start, end};
		}
		return {0, 0};
	}

	void onBarObjectButtonHover(CCObject* sender, CCPoint point, bool hovering, bool isStart) {
		Mod* mod = Mod::get();
		
		if (isStart && hovering) {
			
			if (mod->getSettingValue<bool>("enable-new-tab-ui")) {
				if (ButtonSprite* buttonSprite = static_cast<CCNode*>(sender)->getChildByType<ButtonSprite*>(0)) {
					buttonSprite->getChildByID("highlight")->setVisible(true);
				}
			}

			std::string name;
			if (sender->getTag() < 0) {
				name = "Custom Object";
			}
			else if (sender->getTag() > 0) {
				name = ObjectNames::get()->nameForID(sender->getTag());
			}
			
			if (mod->getSettingValue<bool>("enable-tooltips")) {
				bool isObjectGroup = false;
				if (CCNode* parent = static_cast<CCNode*>(sender)->getParent()) {
					if (CCNode* parent2 = parent->getParent()) {
						isObjectGroup = parent2->getID() == "RaZooM";
					}
				}
				setTooltipText(name, sender->getTag());
				if (!isObjectGroup) {
					setTooltipVisible(true);
				}
			}
		}
		if (isStart && !hovering) {
			if (mod->getSettingValue<bool>("enable-new-tab-ui")) {
				if (ButtonSprite* buttonSprite = static_cast<CCNode*>(sender)->getChildByType<ButtonSprite*>(0)) {
					buttonSprite->getChildByID("highlight")->setVisible(false);
				}
			}
			if (mod->getSettingValue<bool>("enable-tooltips")) {
				setTooltipVisible(false);
			}
		}
		if (hovering) {
			if (mod->getSettingValue<bool>("enable-tooltips")) {
				setTooltipPosition(point);
			}
		}
	}

    CreateMenuItem* getCreateBtn(int id, int bg) {
		auto ret = EditorUI::getCreateBtn(id, bg);
    	Mod* mod = Mod::get();
        ret->setTag(ret->m_objectID);

        static_cast<HoverEnabledCCMenuItemSpriteExtra*>(static_cast<CCMenuItemSpriteExtra*>(ret))->enableHover(std::bind(&MyEditorUI::onBarObjectButtonHover, this, _1, _2, _3, _4));

    	if (mod->getSettingValue<bool>("enable-new-tab-ui")) {
			ret->m_scaleMultiplier = 1;
			if (ButtonSprite* buttonSprite = ret->getChildByType<ButtonSprite*>(0)) {
				if (buttonSprite->m_BGSprite) {
					buttonSprite->m_BGSprite->setVisible(false);
				}
				else {
					for (CCNode* child : CCArrayExt<CCNode*>(buttonSprite->getChildren())) {
						if (typeinfo_cast<CCSprite*>(child)) {
							if (GameObject* obj = typeinfo_cast<GameObject*>(child)) {
								obj->retain();
								queueInMainThread([this, obj] {
									setColorRecursive(obj);
									obj->release();
								});
								continue;
							}
							if (!child->getID().empty()) continue;
							if (child->getContentSize() == CCSize{40, 40}) {
								child->setVisible(false);
								break;
							}
						}
					}
				}

				if (buttonSprite->getChildByID("slot-bg")) return ret;
				
				CCSprite* slotSprite = CCSprite::create("rounded-slot.png"_spr);
				CCSprite* slotOverlay = CCSprite::create("rounded-slot-overlay.png"_spr);
				slotSprite->setZOrder(-10);
				slotSprite->setID("slot-bg");
				slotOverlay->setZOrder(10);
				slotOverlay->setVisible(false);
				slotOverlay->setID("slot-overlay");

				ccColor3B overlayColor = mod->getSettingValue<ccColor3B>("selection-color");
				slotOverlay->setColor(overlayColor);

				CCSprite* highlight = CCSprite::create("rounded-slot-highlight.png"_spr);
				highlight->setVisible(false);
				highlight->setID("highlight");
				highlight->setZOrder(-11);
				highlight->setOpacity(127);

				CCSprite* persistentHighlight = CCSprite::create("rounded-slot-highlight.png"_spr);
				persistentHighlight->setID("persistent-highlight");
				persistentHighlight->setZOrder(-12);
				persistentHighlight->setOpacity(127);
				persistentHighlight->setColor({0, 150, 255});

				buttonSprite->addChild(slotSprite);
				buttonSprite->addChild(slotOverlay);
				buttonSprite->addChild(highlight);

				slotSprite->setPosition(buttonSprite->getContentSize()/2);
				slotSprite->setPositionY(slotSprite->getPositionY() + 2);
				slotSprite->setScale(buttonSprite->getContentSize().width / slotSprite->getContentSize().width);
				slotOverlay->setPosition(buttonSprite->getContentSize()/2);
				slotOverlay->setPositionY(slotOverlay->getPositionY() + 2);
				slotOverlay->setScale(buttonSprite->getContentSize().width / slotOverlay->getContentSize().width);

				highlight->setPosition(buttonSprite->getContentSize()/2);
				highlight->setPositionY(highlight->getPositionY() + 2);
				highlight->setScale(buttonSprite->getContentSize().width / highlight->getContentSize().width);

				persistentHighlight->setPosition(buttonSprite->getContentSize()/2);
				persistentHighlight->setPositionY(persistentHighlight->getPositionY() + 2);
				persistentHighlight->setScale(buttonSprite->getContentSize().width / persistentHighlight->getContentSize().width);

				ret->retain();
				buttonSprite->retain();
				persistentHighlight->retain();
				queueInMainThread([ret, buttonSprite, persistentHighlight] {
					if (typeinfo_cast<GroupInfo*>(ret->getUserObject())) {
						buttonSprite->addChild(persistentHighlight);
					}
					ret->release();
					buttonSprite->release();
					persistentHighlight->release();
				});
			}
		}
		
		return ret;
	}

	ccColor3B cc4BTo3B(ccColor4B color) {
		return {color.r, color.g, color.b};
	}

	CCNode* createCoolBG(int opacity, int outlineOpacity, int overlayOpacity) {

		CCNode* containerNode = CCNode::create();

		Mod* mod = Mod::get();

		CCSprite* outlineHalf1 = CCSprite::create("GJ_square07.png");
		CCSprite* outlineHalf2 = CCSprite::create("GJ_square07.png");

		CCSprite* innerPartHalf1 = CCSprite::create("square02b_001.png");
		CCSprite* innerPartHalf2 = CCSprite::create("square02b_001.png");

		CCSprite* innerPartOverlayHalf1 = CCSprite::create("square02b_001.png");
		CCSprite* innerPartOverlayHalf2 = CCSprite::create("square02b_001.png");

		float scaleFactor = 3.f;

		ccColor3B lineColor = mod->getSettingValue<ccColor3B>("line-color");

		outlineHalf1->setTextureRect({outlineHalf1->getContentWidth() - outlineHalf1->getContentWidth()/scaleFactor, 0, outlineHalf1->getContentWidth()/scaleFactor, outlineHalf1->getContentHeight()/scaleFactor});
		outlineHalf1->setAnchorPoint({0, 0});
		outlineHalf1->setPositionX(outlineHalf1->getContentWidth());
		outlineHalf1->setOpacity(outlineOpacity);
		outlineHalf1->setColor(lineColor);

		outlineHalf2->setTextureRect({0, 0, outlineHalf2->getContentWidth()/scaleFactor, outlineHalf2->getContentHeight()/scaleFactor});
		outlineHalf2->setAnchorPoint({0, 0});
		outlineHalf2->setOpacity(outlineOpacity);
		outlineHalf2->setColor(lineColor);

		innerPartHalf1->setTextureRect({innerPartHalf1->getContentWidth() - innerPartHalf1->getContentWidth()/scaleFactor, 0, innerPartHalf1->getContentWidth()/scaleFactor, innerPartHalf1->getContentHeight()/scaleFactor});
		innerPartHalf1->setOpacity(opacity);
		innerPartHalf1->setPositionX(innerPartHalf1->getContentWidth());
		innerPartHalf1->setAnchorPoint({0, 0});
		innerPartHalf1->setColor(cc4BTo3B(mod->getSettingValue<ccColor4B>("gradient-top-color")));
		innerPartHalf1->setZOrder(-2);
		innerPartHalf1->setBlendFunc({GL_SRC_ALPHA, GL_ONE});

		innerPartHalf2->setTextureRect({0, 0, innerPartHalf2->getContentWidth()/scaleFactor, innerPartHalf2->getContentHeight()/scaleFactor});
		innerPartHalf2->setOpacity(opacity);
		innerPartHalf2->setAnchorPoint({0, 0});
		innerPartHalf2->setColor(cc4BTo3B(mod->getSettingValue<ccColor4B>("gradient-top-color")));
		innerPartHalf2->setZOrder(-2);
		innerPartHalf2->setBlendFunc({GL_SRC_ALPHA, GL_ONE});

		innerPartOverlayHalf1->setTextureRect({innerPartOverlayHalf1->getContentWidth() - innerPartOverlayHalf1->getContentWidth()/scaleFactor, 0, innerPartOverlayHalf1->getContentWidth()/scaleFactor, innerPartOverlayHalf1->getContentHeight()/scaleFactor});
		innerPartOverlayHalf1->setPositionX(innerPartOverlayHalf1->getContentWidth());
		innerPartOverlayHalf1->setAnchorPoint({0, 0});
		innerPartOverlayHalf1->setColor({0, 0, 0});
		innerPartOverlayHalf1->setOpacity(overlayOpacity);
		innerPartOverlayHalf1->setZOrder(-1);

		innerPartOverlayHalf2->setTextureRect({0, 0, innerPartOverlayHalf2->getContentWidth()/scaleFactor, innerPartOverlayHalf2->getContentHeight()/scaleFactor});
		innerPartOverlayHalf2->setAnchorPoint({0, 0});
		innerPartOverlayHalf2->setColor({0, 0, 0});
		innerPartOverlayHalf2->setOpacity(overlayOpacity);
		innerPartOverlayHalf2->setZOrder(-1);

		containerNode->setZOrder(-1);

		containerNode->addChild(outlineHalf1);
		containerNode->addChild(outlineHalf2);

		containerNode->addChild(innerPartHalf1);
		containerNode->addChild(innerPartHalf2);
		
		containerNode->addChild(innerPartOverlayHalf1);
		containerNode->addChild(innerPartOverlayHalf2);

		containerNode->setAnchorPoint({0, 0});
		containerNode->setScale(0.6f);

		return containerNode;
	}

	void setLines(EditorUI* editorUI) {
		if (Mod::get()->getSettingValue<bool>("enable-new-tab-ui")) {
			auto fields = static_cast<MyEditorUI*>(editorUI)->m_fields.self();
			CCSize winSize = CCDirector::get()->getWinSize();

			std::pair<float, float> edges = getSelectedButtonEdges(editorUI);

			if (fields->m_linePart1) fields->m_linePart1->setContentWidth(edges.first);
			if (fields->m_linePart2) fields->m_linePart2->setContentWidth(winSize.width - edges.second);
		}
	}

	void changeBG(CCNode* node, int opacity, int outlineOpacity, int overlayOpacity) {
		for (CCMenuItemSpriteExtra* toggles : CCArrayExt<CCMenuItemSpriteExtra*>(node->getChildren())) {
			if (CCSprite* bg = toggles->getChildByType<CCSprite*>(0)) {
				bg->setOpacity(0);
				if (CCNode* icon = bg->getChildByType<CCNode*>(0)) {
					if (icon->getTag() != 1) {
						icon->setTag(1);
						icon->setScale(icon->getScale() * 0.8f);
					}
				}
			}
			toggles->m_scaleMultiplier = 1;
			toggles->removeChildByID("background"_spr);
			CCNode* newBG = createCoolBG(opacity, outlineOpacity, overlayOpacity);
			newBG->setID("background"_spr);
			toggles->addChild(newBG);
		}
	}

	void selectBuildTab(int tab) {
		EditorUI::selectBuildTab(tab);
		Mod* mod = Mod::get();
		if (mod->getSettingValue<bool>("enable-new-tab-ui")) {
			setLines(this);
			CCNode* currentTab = m_tabsMenu->getChildByTag(m_selectedTab);

			if (m_selectedMode == 2) {
				for (CCNode* tab : CCArrayExt<CCNode*>(m_tabsMenu->getChildren())) {
					changeBG(tab, 127, 172, 86);
				}
				changeBG(currentTab, mod->getSettingValue<ccColor4B>("gradient-top-color").a, 255, 255 * mod->getSettingValue<float>("gradient-overlay-opacity"));
			}
			HoverEnabledCCMenuItemSpriteExtra* btn = static_cast<HoverEnabledCCMenuItemSpriteExtra*>(currentTab);
			if (CCFloat* yFloat = static_cast<CCFloat*>(btn->getUserObject("orig-y"_spr))) {
				float origY = yFloat->getValue();
				if (currentTab->getTag() == m_selectedTab) {
					btn->setPositionY(origY);
				}
			}
		}
		setTooltipVisible(false);
	}

	void setTooltipText(std::string text, int id) {
		auto fields = m_fields.self();
		if (fields->m_tooltipText) {
			if (id == 0) {
				fields->m_tooltipText->setString("");
				return;
			}
			fields->m_tooltipText->setString(text.c_str());
			id = std::abs(id);
			fields->m_tooltipText->setString(text.c_str());
			fields->m_tooltipObjID->setString(fmt::format("(#{})", id).c_str());

			float scaleFactor = 4.f;

			fields->m_tooltip->setContentSize({fields->m_tooltipText->getScaledContentSize().width + 10, fields->m_tooltipText->getScaledContentSize().height + fields->m_tooltipObjID->getScaledContentSize().height + 5});
			fields->m_tooltipBG->setContentSize(fields->m_tooltip->getContentSize() * scaleFactor);
			fields->m_tooltipBG->setScale(1 / scaleFactor);

			fields->m_tooltipText->setPositionY(fields->m_tooltip->getContentSize().height - 2);
			fields->m_tooltipObjID->setPositionY(3);
		}
	}

	void setTooltipPosition(CCPoint point) {
		auto fields = m_fields.self();

		CCSize winSize = CCDirector::get()->getWinSize();

		if (fields->m_tooltip->getScaledContentSize().width + point.x > winSize.width) {
			point.x = winSize.width - fields->m_tooltip->getScaledContentSize().width;
		}

		if (fields->m_tooltip) fields->m_tooltip->setPosition(point);
	}

	void setTooltipVisible(bool visible) {
		auto fields = m_fields.self();
		if (!fields->m_tooltipText || !fields->m_tooltip) return;
    	if (std::string_view(fields->m_tooltipText->getString()).empty()) return;

		if (!fields->m_queueVisible && visible) {
			fields->m_queueVisible = true;
			fields->m_tooltip->retain();
			queueInMainThread([this, fields] {
				fields->m_tooltip->setVisible(true);
				fields->m_tooltip->release();
				fields->m_queueVisible = false;
			});
		}
		
		fields->m_tooltip->setVisible(visible);
	}

    void toggleMode(cocos2d::CCObject* sender) {
		EditorUI::toggleMode(sender);
		auto fields = m_fields.self();
		if (fields->m_creativeMenu) fields->m_creativeMenu->setVisible(m_selectedMode == 2);
		setLines(this);
		setTooltipVisible(false);
	}

	#ifdef GEODE_IS_MACOS
	void onPlaytest(CCObject* sender) {
		auto playbackMode = m_editorLayer->m_playbackMode;
		EditorUI::onPlaytest(sender);
		if (!m_isPaused && playbackMode != PlaybackMode::Playing) {
			if (m_fields->m_creativeMenu) m_fields->m_creativeMenu->setVisible(m_selectedMode == 2);
			setLines(this);
			setTooltipVisible(false);
		}
	}
	#endif

	void showUI(bool show) {
		EditorUI::showUI(show);
		auto fields = m_fields.self();
		fields->m_creativeMenu->setVisible(show && m_selectedMode == 2);
		if (Mod::get()->getSettingValue<bool>("enable-new-tab-ui")) {
			fields->m_gradientBG->setVisible(false);
			fields->m_newGradientBG->setVisible(show);
			fields->m_lineNode->setVisible(show);
			fields->m_darkenedBG->setVisible(show);
			if (fields->m_blur) fields->m_blur->getBlurredSprite()->setVisible(show);
		}
		setTooltipVisible(false);
	}

	void onCreativeMenu(CCObject* sender) {
		auto fields = m_fields.self();

		fields->m_objectSelectPopup = ObjectSelectPopup::create(this);
		fields->m_objectSelectPopup->show();
	}

	void updateCreateMenu(bool p0) {
		EditorUI::updateCreateMenu(p0);
		if (Mod::get()->getSettingValue<bool>("enable-new-tab-ui")) {
			for (CreateMenuItem* item : CCArrayExt<CreateMenuItem*>(m_createButtonArray)) {
				if (ButtonSprite* buttonSprite = item->getChildByType<ButtonSprite*>(0)) {
					if (CCNode* overlay = buttonSprite->getChildByID("slot-overlay")) {
						overlay->setVisible(item->m_objectID == m_selectedObjectIndex);
					}
					if (item->m_objectID == m_selectedObjectIndex) {
						if (GameObject* obj = buttonSprite->getChildByType<GameObject*>(0)) {
							fixColorRecursive(obj);
						}
					}
				}
			}
			for (CreateMenuItem* item : CCArrayExt<CreateMenuItem*>(m_customObjectButtonArray)) {
				if (ButtonSprite* buttonSprite = item->getChildByType<ButtonSprite*>(0)) {
					if (CCNode* overlay = buttonSprite->getChildByID("slot-overlay")) {
						overlay->setVisible(item->m_objectID == m_selectedObjectIndex);
					}
				}
			}
		}
	}

	void setColorRecursive(CCNode* node) {
		if (CCNodeRGBA* nodeRGBA = typeinfo_cast<CCNodeRGBA*>(node)) {
			nodeRGBA->setUserObject("color"_spr, ObjWrapper<ccColor3B>::create(nodeRGBA->getColor()));
		}
		for (CCNode* child : CCArrayExt<CCNode*>(node->getChildren())) {
			setColorRecursive(child);
		}
	}

	void fixColorRecursive(CCNode* node) {
		if (CCNodeRGBA* nodeRGBA = typeinfo_cast<CCNodeRGBA*>(node)) {
			if (ObjWrapper<ccColor3B>* color = static_cast<ObjWrapper<ccColor3B>*>(nodeRGBA->getUserObject("color"_spr))) {
				nodeRGBA->setColor(color->getValue());
			}
		}
		for (CCNode* child : CCArrayExt<CCNode*>(node->getChildren())) {
			fixColorRecursive(child);
		}
	}

	void onCreateButton(CCObject* sender) {
		EditorUI::onCreateButton(sender);
		if (Mod::get()->getSettingValue<bool>("enable-new-tab-ui")) {
			CreateMenuItem* btn = static_cast<CreateMenuItem*>(sender);
			if (ButtonSprite* buttonSprite = btn->getChildByType<ButtonSprite*>(0)) {
				if (GameObject* obj = buttonSprite->getChildByType<GameObject*>(0)) {
					fixColorRecursive(obj);
				}
			}
		}
	}

	void onObjectButton(CCObject* sender) {
		auto fields = m_fields.self();
		int id = sender->getTag();

		HoverEnabledCCMenuItemSpriteExtra* btn = static_cast<HoverEnabledCCMenuItemSpriteExtra*>(sender);
		CCSprite* overlay = static_cast<CCSprite*>(btn->getChildByID("slot-overlay"));
		onCreateButton(sender);

		m_selectedObjectIndex = id;
		selectBuildTab(static_cast<CCInteger*>(btn->getUserObject("tab"_spr))->getValue());
		updateCreateMenu(true);

		for (CCMenuItem* btn : fields->m_objectSelectPopup->m_buttons) {
			CCSprite* overlay2 = static_cast<CCSprite*>(btn->getChildByID("slot-overlay"));
			overlay2->setVisible(btn->getTag() == id);
		}
		
		overlay->setVisible(true);
	}

	void onSearchHover(CCObject* sender, CCPoint point, bool hovering, bool isStart) {
		HoverEnabledCCMenuItemSpriteExtra* btn = static_cast<HoverEnabledCCMenuItemSpriteExtra*>(sender);
		if (isStart && hovering) {
			static_cast<CCSprite*>(btn->getChildByID("highlight"))->setOpacity(127);
		}
		if (isStart && !hovering) {
			static_cast<CCSprite*>(btn->getChildByID("highlight"))->setOpacity(10);
		}
	}

	void onTabHover(CCObject* sender, CCPoint point, bool hovering, bool isStart) {
		HoverEnabledCCMenuItemSpriteExtra* btn = static_cast<HoverEnabledCCMenuItemSpriteExtra*>(sender);
		float origY = static_cast<CCFloat*>(btn->getUserObject("orig-y"_spr))->getValue();
		if (sender->getTag() == m_selectedTab) {
			btn->setPositionY(origY);
		}
		else {
			if (isStart && hovering) {
				btn->stopActionByTag(2);
				bool doAction = true;
				if (CCAction* action = btn->getActionByTag(1)) {
					doAction = action->isDone();
				}
				if (doAction) {
					CCMoveTo* moveUp = CCMoveTo::create(0.1f, {btn->getPositionX(), origY + 1});
					moveUp->setTag(1);
					btn->runAction(moveUp);
				}
			}
			if (!hovering) {
				btn->stopActionByTag(1);
				bool doAction = true;
				if (CCAction* action = btn->getActionByTag(2)) {
					doAction = action->isDone();
				}
				if (doAction) {
					CCMoveTo* moveDown = CCMoveTo::create(0.1f, {btn->getPositionX(), origY});
					moveDown->setTag(2);
					btn->runAction(moveDown);
				}
			}
		}
	}

	void onObjectButtonHover(CCObject* sender, CCPoint point, bool hovering, bool isStart) {
		HoverEnabledCCMenuItemSpriteExtra* btn = static_cast<HoverEnabledCCMenuItemSpriteExtra*>(sender);
		auto fields = m_fields.self();

		if (isStart && hovering) {
			std::string name;
			if (btn->getTag() < 0) {
				name = "Custom Object";
			}
			else if (btn->getTag() > 0) {
				name = ObjectNames::get()->nameForID(btn->getTag());
			}
			fields->m_objectSelectPopup->setTooltipText(name, btn->getTag());
			fields->m_objectSelectPopup->setTooltipVisible(true);
			btn->getChildByID("highlight")->setVisible(true);
		}
		if (isStart && !hovering) {
			fields->m_objectSelectPopup->setTooltipVisible(false);
			btn->getChildByID("highlight")->setVisible(false);
		}

		if (hovering) {
			fields->m_objectSelectPopup->setTooltipPosition(point);
		}
	}

	GameObject* createGameObject(int id, MyEditorUI::Fields* fields) {
		if (id == 0) return nullptr;
		if (fields->m_gameObjects.contains(id)) {
			return fields->m_gameObjects[id];
		}
		else {
			CreateMenuItem* cmi = getCreateBtn(id, 0);
			ButtonSprite* buttonSprite = cmi->getChildByType<ButtonSprite*>(0);
			GameObject* obj = buttonSprite->getChildByType<GameObject*>(0);
			fields->m_gameObjects[id] = obj;
			return obj;
		}
		return nullptr;
	}

	HoverEnabledCCMenuItemSpriteExtra* createObjectButton(int id, int tab, MyEditorUI::Fields* fields) {
		Mod* mod = Mod::get();
		CCNode* objectContainer = CCNode::create();
		CCSprite* slotSprite = CCSprite::create("rounded-slot.png"_spr);
		CCSprite* slotOverlay = CCSprite::create("rounded-slot-overlay.png"_spr);
		slotSprite->setZOrder(-10);
		slotOverlay->setZOrder(10);
		slotOverlay->setVisible(false);
		slotOverlay->setID("slot-overlay");
		
		ccColor3B overlayColor = mod->getSettingValue<ccColor3B>("selection-color");
		slotOverlay->setColor(overlayColor);

		CCSprite* highlight = CCSprite::create("rounded-slot-highlight.png"_spr);
		highlight->setVisible(false);
		highlight->setID("highlight");
		highlight->setZOrder(-11);
		highlight->setOpacity(127);

		objectContainer->setAnchorPoint({1, 0.5});
		objectContainer->setScale(ObjectSelectPopup::s_scaleMult * 0.7);
		if (GameObject* obj = createGameObject(id, fields)) {
			objectContainer->addChild(obj);
		}

		objectContainer->setContentSize({40, 40});
		CCMenuItemSpriteExtra* btn = CCMenuItemSpriteExtra::create(objectContainer, this, menu_selector(MyEditorUI::onObjectButton));
		HoverEnabledCCMenuItemSpriteExtra* hoverBtn = static_cast<HoverEnabledCCMenuItemSpriteExtra*>(btn);
		hoverBtn->enableHover(std::bind(&MyEditorUI::onObjectButtonHover, this, _1, _2, _3, _4));
		objectContainer->setPosition({objectContainer->getPositionX() + 2.75f * objectContainer->getScale(), objectContainer->getPositionY() + 1 * objectContainer->getScale()});
        btn->m_scaleMultiplier = 1;
		btn->addChild(slotSprite);
		btn->addChild(slotOverlay);
		btn->addChild(highlight);
		btn->setContentSize({25.6, 25.6});

		btn->setTag(id);
		btn->setUserObject("tab"_spr, CCInteger::create(tab));
		slotSprite->setPosition(btn->getContentSize()/2);
		slotSprite->setScale(btn->getContentSize().width / slotSprite->getContentSize().width);
		slotOverlay->setPosition(btn->getContentSize()/2);
		slotOverlay->setScale(btn->getContentSize().width / slotSprite->getContentSize().width);
		highlight->setPosition(btn->getContentSize()/2);
		highlight->setScale(btn->getContentSize().width / slotSprite->getContentSize().width);

		return hoverBtn;
	}
};