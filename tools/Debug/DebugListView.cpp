#include "DebugListView.hpp"

DebugCell::DebugCell(const char* name, CCSize size) :
    TableViewCell(name, size.width, size.height) {}

void DebugCell::draw() {
    // just call StatsCell::draw, no one will notice
    reinterpret_cast<void(__thiscall*)(DebugCell*)>(
        base + 0x59d40
    )(this);
}

void DebugCell::updateBGColor(int ix) {
    this->m_pBGLayer->setOpacity(ix % 2 ? 150 : 50);
    this->m_pBGLayer->setColor({ 0, 0, 0 });
}

void DebugCell::loadFromObject(DebugObject* str) {
    this->m_pLayer->setVisible(true);
    this->m_pBGLayer->setOpacity(255);

    bool showMore = false;
    auto text = str->m_sString;
    if (text.size() > 72) {
        text = text.substr(0, 72) + "...";
        showMore = true;
    }
    this->m_sText = str->m_sString;
    
    auto label = CCLabelBMFont::create(text.c_str(), "chatFont.fnt");
    label->setAnchorPoint({ .0f, .5f });
    label->setPosition({
        this->m_fHeight / 2,
        this->m_fHeight / 2
    });
    label->setScale(.5f);
    this->m_pLayer->addChild(label);

    this->m_pLabel = label;

    if (showMore) {
        auto menu = CCMenu::create();

        auto moreLabel = CCLabelBMFont::create("More", "chatFont.fnt");
        moreLabel->setScale(.5f);
        moreLabel->setColor(cc3x(0x7bf));
        
        auto moreBtn = CCMenuItemSpriteExtra::create(
            moreLabel, this, menu_selector(DebugCell::onMore)
        );
        moreBtn->setPosition(0, 0);
        menu->addChild(moreBtn);

        menu->setPosition(
            this->m_fHeight / 2 +
            label->getScaledContentSize().width +
            moreLabel->getScaledContentSize().width / 2 + 10.f,
            this->m_fHeight / 2
        );

        this->m_pLayer->addChild(menu);

        this->registerWithTouchDispatcher();
        CCDirector::sharedDirector()->getTouchDispatcher()->incrementForcePrio(2);
    }

    auto typeLabel = CCLabelBMFont::create(DebugTypeToStr(str->m_eDebugType).c_str(), "chatFont.fnt");
    typeLabel->setAnchorPoint({ 1.f, .5f });
    typeLabel->setPosition({
        this->m_fWidth - this->m_fHeight / 2,
        this->m_fHeight / 2
    });
    typeLabel->setScale(.5f);
    typeLabel->setColor({ 80, 255, 160 });
    this->m_pLayer->addChild(typeLabel);

    this->updateLabelColor();
}

void DebugCell::updateLabelColor() {
    auto str = this->m_pLabel->getString();
    ccColor3B col;
    CCARRAY_FOREACH_B_BASE(this->m_pLabel->getChildren(), child, CCSprite*, ix) {
        switch (str[ix]) {
            case '[': col = { 100, 100, 100 }; break;
        }
        child->setColor(col);
        switch (str[ix]) {
            case ']': col = { 255, 255, 255 }; break;
        }
    }
}

void DebugCell::onMore(CCObject*) {
    FLAlertLayer::create(
        nullptr,
        "Log Message",
        "OK", nullptr,
        340.f,
        this->m_sText
    )->show();
}

DebugCell* DebugCell::create(const char* key, CCSize size) {
    auto pRet = new DebugCell(key, size);

    if (pRet) {
        return pRet;
    }

    CC_SAFE_DELETE(pRet);
    return nullptr;
}



void DebugListView::setupList() {
    this->m_fItemSeparation = 16.0f;

    if (!this->m_pEntries->count()) return;

    this->m_pTableView->reloadData();

    if (this->m_pEntries->count() == 1)
        this->m_pTableView->moveToTopWithOffset(this->m_fItemSeparation);
    
    this->m_pTableView->moveToTop();
}

TableViewCell* DebugListView::getListCell(const char* key) {
    return DebugCell::create(key, { this->m_fWidth, this->m_fItemSeparation });
}

void DebugListView::loadCell(TableViewCell* cell, unsigned int index) {
    as<DebugCell*>(cell)->loadFromObject(
        as<DebugObject*>(this->m_pEntries->objectAtIndex(index))
    );
    as<DebugCell*>(cell)->updateBGColor(index);
}

DebugListView* DebugListView::create(
    CCArray* actions,
    float width,
    float height
) {
    auto pRet = new DebugListView;

    if (pRet) {
        if (pRet->init(actions, kBoomListType_Debug, width, height)) {
            pRet->autorelease();
            return pRet;
        }
    }

    CC_SAFE_DELETE(pRet);
    return nullptr;
}
