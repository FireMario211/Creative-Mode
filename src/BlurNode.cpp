#include "BlurNode.hpp"

BlurNode* BlurNode::create(CCNode* target, float radius) {
    auto ret = new BlurNode();
    if (ret->init(target, radius)) {
        ret->autorelease();
        return ret;
    }

    delete ret;
    return nullptr;
}

bool BlurNode::init(CCNode* target, float radius) {
    m_target = target;
    m_radius = radius;
    CCSize size = CCDirector::get()->getWinSize();
    CCSize sizePixels = CCDirector::get()->getWinSizeInPixels();
    float stepSize = m_radius / 9;
    CCSize distVert = CCSize{0, 1 / size.height} * stepSize;
    CCSize distHorz = CCSize{1 / size.width, 0} * stepSize;

    m_distOffsetsVert[0] = CCSize(-4.0f * distVert.width, -4.0f * distVert.height);
    m_distOffsetsVert[1] = CCSize(-3.0f * distVert.width, -3.0f * distVert.height);
    m_distOffsetsVert[2] = CCSize(-2.0f * distVert.width, -2.0f * distVert.height);
    m_distOffsetsVert[3] = CCSize(-1.0f * distVert.width, -1.0f * distVert.height);
    m_distOffsetsVert[4] = CCSize(0.0f, 0.0f);
    m_distOffsetsVert[5] = CCSize(1.0f * distVert.width, 1.0f * distVert.height);
    m_distOffsetsVert[6] = CCSize(2.0f * distVert.width, 2.0f * distVert.height);
    m_distOffsetsVert[7] = CCSize(3.0f * distVert.width, 3.0f * distVert.height);
    m_distOffsetsVert[8] = CCSize(4.0f * distVert.width, 4.0f * distVert.height);

    m_distOffsetsHorz[0] = CCSize(-4.0f * distHorz.width, -4.0f * distHorz.height);
    m_distOffsetsHorz[1] = CCSize(-3.0f * distHorz.width, -3.0f * distHorz.height);
    m_distOffsetsHorz[2] = CCSize(-2.0f * distHorz.width, -2.0f * distHorz.height);
    m_distOffsetsHorz[3] = CCSize(-1.0f * distHorz.width, -1.0f * distHorz.height);
    m_distOffsetsHorz[4] = CCSize(0.0f, 0.0f);
    m_distOffsetsHorz[5] = CCSize(1.0f * distHorz.width, 1.0f * distHorz.height);
    m_distOffsetsHorz[6] = CCSize(2.0f * distHorz.width, 2.0f * distHorz.height);
    m_distOffsetsHorz[7] = CCSize(3.0f * distHorz.width, 3.0f * distHorz.height);
    m_distOffsetsHorz[8] = CCSize(4.0f * distHorz.width, 4.0f * distHorz.height);

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
    CCDirector::get()->getScheduler()->scheduleSelector(schedule_selector(BlurNode::update), this, 0, false);
    return true;
}

void BlurNode::initShader() {
    m_program = new CCGLProgram();
    m_program->initWithVertexShaderFilename("Default.vsh"_spr, "Blur.fsh"_spr);
    m_program->addAttribute(kCCAttributeNamePosition, kCCVertexAttrib_Position);
    m_program->addAttribute(kCCAttributeNameTexCoord, kCCVertexAttrib_TexCoords);
    m_program->link();
    m_program->updateUniforms();
    m_program->autorelease();
}

void BlurNode::applyShader(CCSprite* sprite, int direction) {
    #ifndef GEODE_IS_MACOS
    m_program->use();

    m_program->setUniformLocationWith2f(
        m_program->getUniformLocationForName("u_resolution"),
        sprite->getContentSize().width,
        sprite->getContentSize().height
    );
    m_program->setUniformLocationWith2fv(
        m_program->getUniformLocationForName("u_distOffsets"),
        (GLfloat*) (direction ? m_distOffsetsVert : m_distOffsetsHorz),
        9
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

void BlurNode::update(float dt) {
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

void BlurNode::setCrop(CCRect crop) {
    m_crop = crop;
}

void BlurNode::addNodeToIgnore(CCNode* node) {
    if(std::find(m_nodesToIgnore.begin(), m_nodesToIgnore.end(), node) == m_nodesToIgnore.end()) {
        m_nodesToIgnore.push_back(node);
    }
}

void BlurNode::addNodeToVisit(CCNode* node) {
    m_nodesToVisit.push_back(node);
}

CCSprite* BlurNode::getBlurredSprite() {
    return m_blurredFinalSprite->sprite;
}
