#include "BoxBlurEffect.hpp"

BoxBlurEffect* BoxBlurEffect::create(CCNode* target, float radius) {
    auto ret = new BoxBlurEffect();
    if (ret->init(target, radius)) {
        ret->autorelease();
        return ret;
    }

    delete ret;
    return nullptr;
}

bool BoxBlurEffect::init(CCNode* target, float radius) {
    m_target = target;
    m_radius = radius;
    CCSize size = CCDirector::get()->getWinSize();
    CCSize sizePixels = CCDirector::get()->getWinSizeInPixels();
    m_crop = {0, 0, size.width, size.height};

    m_blurredVertSprite = std::make_shared<RenderTexture::Sprite>(RenderTexture((unsigned int)sizePixels.width, (unsigned int)sizePixels.height));
    m_blurredVertSprite->sprite->setFlipY(true);
    m_blurredVertSprite->sprite->ignoreAnchorPointForPosition(true);
    m_blurredVertSprite->sprite->setAnchorPoint({0, 0});
    m_blurredVertSprite->sprite->setZOrder(-100);

    m_blurredHorizSprite = std::make_shared<RenderTexture::Sprite>(RenderTexture((unsigned int)sizePixels.width, (unsigned int)sizePixels.height));
    m_blurredHorizSprite->sprite->setFlipY(true);
    m_blurredHorizSprite->sprite->ignoreAnchorPointForPosition(true);
    m_blurredHorizSprite->sprite->setAnchorPoint({0, 0});
    m_blurredHorizSprite->sprite->setZOrder(-100);

    m_blurredFinalSprite = std::make_shared<RenderTexture::Sprite>(RenderTexture((unsigned int)sizePixels.width, (unsigned int)sizePixels.height));
    m_blurredFinalSprite->sprite->setFlipY(true);
    m_blurredFinalSprite->sprite->ignoreAnchorPointForPosition(true);
    m_blurredFinalSprite->sprite->setAnchorPoint({0, 0});
    m_blurredFinalSprite->sprite->setZOrder(-100);

    initShader();
    CCDirector::get()->getScheduler()->scheduleSelector(schedule_selector(BoxBlurEffect::update), this, 0, false);
    return true;
}

void BoxBlurEffect::initShader() {
    m_program = new CCGLProgram();
    m_program->initWithVertexShaderFilename("Default.vsh"_spr, "BoxBlur.fsh"_spr);
    m_program->addAttribute(kCCAttributeNamePosition, kCCVertexAttrib_Position);
    m_program->addAttribute(kCCAttributeNameTexCoord, kCCVertexAttrib_TexCoords);
    m_program->link();
    m_program->updateUniforms();
    m_program->autorelease();
}

void BoxBlurEffect::applyShader(CCSprite* sprite, int direction) {
    #ifndef GEODE_IS_MACOS
    m_program->use();

    m_program->setUniformLocationWith2f(
        m_program->getUniformLocationForName("u_resolution"),
        sprite->getContentSize().width,
        sprite->getContentSize().height
    );
    m_program->setUniformLocationWith1f(
        m_program->getUniformLocationForName("u_radius"),
        m_radius
    );
    m_program->setUniformLocationWith1i(
        m_program->getUniformLocationForName("u_direction"),
        direction
    );
    sprite->setShaderProgram(m_program);
    #endif
}

void BoxBlurEffect::update(float dt) {

    for (CCNode* node : m_nodesToIgnore) {
        node->setVisible(false);
    }
    m_blurredHorizSprite->render.begin();
    for (CCNode* node : m_nodesToVisit) {
        node->visit();
    }
    if (!m_target) return;
    m_target->visit();
    m_blurredHorizSprite->render.end();
    
    CCSprite* horizSpr = CCSprite::createWithTexture(m_blurredHorizSprite->sprite->getTexture());
    horizSpr->setAnchorPoint({0, 0});
    horizSpr->setFlipY(true);
    applyShader(horizSpr, 0);
    m_blurredVertSprite->render.capture(horizSpr);

    CCSprite* vertSpr = CCSprite::createWithTexture(m_blurredVertSprite->sprite->getTexture());
    vertSpr->setAnchorPoint({0, 0});
    vertSpr->setFlipY(true);
    applyShader(vertSpr, 1);
    m_blurredFinalSprite->render.capture(vertSpr);

    CCSprite* finalSpr = m_blurredFinalSprite->sprite;
    finalSpr->setTextureRect(m_crop);
    finalSpr->setAnchorPoint({0, 0});
    finalSpr->setFlipY(true);

    for (CCNode* node : m_nodesToIgnore) {
        node->setVisible(true);
    }
}

void BoxBlurEffect::setCrop(CCRect crop) {
    m_crop = crop;
}

void BoxBlurEffect::addNodeToIgnore(CCNode* node) {
    m_nodesToIgnore.push_back(node);
}

void BoxBlurEffect::addNodeToVisit(CCNode* node) {
    m_nodesToVisit.push_back(node);
}

CCSprite* BoxBlurEffect::getBlurredSprite() {
    return m_blurredFinalSprite->sprite;
}
