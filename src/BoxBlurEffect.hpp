#pragma once

#include <Geode/Geode.hpp>
#include "RenderTexture.hpp"

using namespace geode::prelude;

class BoxBlurEffect : public CCNode {
public:
    static BoxBlurEffect* create(CCNode* target, float radius);
    CCSprite* getBlurredSprite();
    virtual void update(float dt) override;
    void addNodeToIgnore(CCNode* node);
    void addNodeToVisit(CCNode* node);
    void setCrop(CCRect crop);

protected:
    bool init(CCNode* target, float radius);

private:
    CCNode* m_target;
    float m_radius;
    std::shared_ptr<RenderTexture::Sprite> m_blurredHorizSprite;
    std::shared_ptr<RenderTexture::Sprite> m_blurredVertSprite;
    std::shared_ptr<RenderTexture::Sprite> m_blurredFinalSprite;
    Ref<CCGLProgram> m_program;
    std::vector<Ref<CCNode>> m_nodesToIgnore;
    std::vector<Ref<CCNode>> m_nodesToVisit;
    CCRect m_crop;

    void initShader();
    void applyShader(CCSprite* sprite, int direction);
};
