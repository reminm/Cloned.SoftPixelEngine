/*
 * GUI tree gadget file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "GUI/spGUITreeGadget.hpp"

#ifdef SP_COMPILE_WITH_GUI


#include "RenderSystem/spRenderSystem.hpp"
#include "GUI/spGUIScrollbarGadget.hpp"
#include "GUI/spGUIManager.hpp"

#include <boost/foreach.hpp>


namespace sp
{

extern video::RenderSystem* GlbRenderSys;
extern gui::GUIManager* GlbGUIMngr;

namespace gui
{


/*
 * GUITreeItem class
 */

GUITreeItem::GUITreeItem() :
    Icon_               (0      ),
    Parent_             (0      ),
    isExpand_           (false  ),
    isVisible_          (true   ),
    isPicked_           (false  ),
    hasExplorerSubDir_  (false  )
{
}
GUITreeItem::~GUITreeItem()
{
}

void GUITreeItem::setParent(GUITreeItem* Parent)
{
    if (Parent_ != Parent)
    {
        if (!Parent)
            Parent_->removeChild(this);
        
        Parent_ = Parent;
        
        if (Parent_)
            Parent_->addChild(this);
    }
}

void GUITreeItem::addChild(GUITreeItem* Child)
{
    Children_.push_back(Child);
}
void GUITreeItem::removeChild(GUITreeItem* Child)
{
    for (std::list<GUITreeItem*>::iterator it = Children_.begin(); it != Children_.end(); ++it)
    {
        if (*it == Child)
        {
            Children_.erase(it);
            break;
        }
    }
}


/*
 * GUITreeGadget class
 */

const video::color GUITreeGadget::ITEMPICK_COLOR_A = video::color(200, 200, 255);
const video::color GUITreeGadget::ITEMPICK_COLOR_B = video::color(120, 120, 170);

GUITreeGadget::GUITreeGadget() :
    GUIGadget           (GADGET_TREE),
    GUIScrollViewBased  (           ),
    SelectedItem_       (0          ),
    MaxItemWidth_       (0          ),
    isExplorer_         (false      )
{
    HorzScroll_.setParent(this);
    VertScroll_.setParent(this);
}
GUITreeGadget::~GUITreeGadget()
{
    clearItems();
}

bool GUITreeGadget::update()
{
    updateScrollBars(&HorzScroll_, &VertScroll_);
    
    if (hasFocus() && GlbGUIMngr->MouseWheel_)
        VertScroll_.scroll(-GlbGUIMngr->MouseWheel_ * 30);
    
    if (!checkDefaultUpdate())
        return false;
    
    if (isEnabled_ && mouseOver(Rect_) && !foreignUsage())
    {
        if (mouseLeft())
            focus();
    }
    
    dim::point2di Pos(getItemsStartPos());
    
    foreach (GUITreeItem* Item, ItemList_)
    {
        if (!Item->getParent())
        {
            if (updateItem(Item, Pos))
                break;
        }
    }
    
    updateChildren();
    
    return true;
}

void GUITreeGadget::draw()
{
    if (!isVisible_ || isValidated_ || !setupClipping())
        return;
    
    GlbRenderSys->draw2DRectangle(Rect_, Color_);
    
    const dim::point2di StartPos(getItemsStartPos());
    dim::point2di Pos(StartPos);
    MaxItemWidth_ = 0;
    
    for (std::list<GUITreeItem*>::iterator it = ItemList_.begin(); it != ItemList_.end(); ++it)
    {
        if (!(*it)->getParent())
            drawItem(*it, Pos);
    }
    
    HorzScroll_.setRange(MaxItemWidth_);
    VertScroll_.setRange(Pos.Y - StartPos.Y);
    
    drawFrame(Rect_, 0, !true);
    
    drawChildren();
}

GUITreeItem* GUITreeGadget::addItem(const io::stringc &Text, GUITreeItem* Parent, video::Texture* Icon)
{
    GUITreeItem* NewItem = MemoryManager::createMemory<GUITreeItem>("GUITreeItem");
    {
        NewItem->setText(Text);
        NewItem->setParent(Parent);
        NewItem->setIcon(Icon);
    }
    ItemList_.push_back(NewItem);
    
    return NewItem;
}
void GUITreeGadget::removeItem(GUITreeItem* Item)
{
    if (SelectedItem_ == Item)
        SelectedItem_ = 0;
    
    MemoryManager::removeElement(ItemList_, Item, true);
}

void GUITreeGadget::clearItems()
{
    MemoryManager::deleteList(ItemList_);
    SelectedItem_ = 0;
}

void GUITreeGadget::setExplorer(bool isExplorer)
{
    if (isExplorer_ == isExplorer)
        return;
    
    clearItems();
    
    isExplorer_ = isExplorer;
    
    if (isExplorer_)
    {
        #if defined(SP_PLATFORM_WINDOWS)
        
        HANDLE SearchHandle;
        WIN32_FIND_DATA FindFileData;
        
        for (c8 i = 'A'; i <= 'Z'; ++i)
        {
            SearchHandle = FindFirstFile((io::stringc(i) + ":/*").c_str(), &FindFileData);
            
            if (SearchHandle != INVALID_HANDLE_VALUE)
                addItem(io::stringc(i) + ":")->hasExplorerSubDir_ = true;
            
            if (SearchHandle)
                FindClose(SearchHandle);
        }
        
        #endif
    }
}

io::stringc GUITreeGadget::getExplorerFullPath(const GUITreeItem* Item) const
{
    if (Item)
    {
        if (Item->getParent())
            return getExplorerFullPath(Item->getParent()) + "/" + Item->getText();
        return Item->getText();
    }
    return "";
}


/*
 * ======= Private: =======
 */

void GUITreeGadget::drawItem(GUITreeItem* Item, dim::point2di &Pos)
{
    if (!Item || !Item->getVisible())
        return;
    
    /* Update item picking */
    if ( Item->isPicked_ || ( (Flags_ & GUIFLAG_HOLDSELECTION) && Item == SelectedItem_ ))
    {
        const dim::rect2di Rect(
            Rect_.Left, Pos.Y, Rect_.Right, Pos.Y + TREEITEM_HEIGHT
        );
        
        GlbRenderSys->draw2DRectangle(
            Rect, ITEMPICK_COLOR_A, ITEMPICK_COLOR_A, ITEMPICK_COLOR_B, ITEMPICK_COLOR_B
        );
        
        Item->isPicked_ = false;
    }
    
    /* Draw the item text */
    drawText(dim::point2di(Pos.X, Pos.Y - 2), Item->getText(), 0, 0);
    
    /* Store the maximal item width */
    const s32 ItemWidth = Font_->getStringWidth(Item->getText()) + Pos.X - Rect_.Left + 5 - ScrollPos_.X;
    if (MaxItemWidth_ < ItemWidth)
        MaxItemWidth_ = ItemWidth;
    
    Pos.Y += TREEITEM_HEIGHT;
    
    if (!Item->Children_.empty() || Item->hasExplorerSubDir_)
    {
        /* Draw the expansion icon and all children */
        drawExpandIcon(dim::point2di(Pos.X - 8, Pos.Y - 9), Item->getExpand());
        
        if (Item->getExpand())
        {
            Pos.X += TREEITEM_EXPAND_SIZE;
            
            for (std::list<GUITreeItem*>::iterator it = Item->Children_.begin(); it != Item->Children_.end(); ++it)
                drawItem(*it, Pos);
            
            Pos.X -= TREEITEM_EXPAND_SIZE;
        }
    }
}

void GUITreeGadget::drawExpandIcon(const dim::point2di &Pos, bool isExpand)
{
    const dim::rect2di Rect(
        Pos.X - EXPANDICON_HALFSIZE - 1, Pos.Y - EXPANDICON_HALFSIZE,
        Pos.X + EXPANDICON_HALFSIZE, Pos.Y + EXPANDICON_HALFSIZE + 1
    );
    
    GlbRenderSys->draw2DRectangle(Rect, 255);
    
    drawFrame(Rect, 0, false);
    
    GlbRenderSys->draw2DLine(
        dim::point2di(Pos.X - EXPANDICON_HALFSIZE + 1, Pos.Y), dim::point2di(Pos.X + EXPANDICON_HALFSIZE - 2, Pos.Y), 0
    );
    
    if (!isExpand)
    {
        GlbRenderSys->draw2DLine(
            dim::point2di(Pos.X, Pos.Y - EXPANDICON_HALFSIZE + 2), dim::point2di(Pos.X, Pos.Y + EXPANDICON_HALFSIZE - 1), 0
        );
    }
}

bool GUITreeGadget::updateItem(GUITreeItem* Item, dim::point2di &Pos)
{
    if (!Item || !Item->getVisible())
        return false;
    
    dim::rect2di Rect(
        Rect_.Left, Pos.Y, Rect_.Right, Pos.Y + TREEITEM_HEIGHT
    );
    
    if (VertScroll_.getVisible())
        Rect.Right -= SCROLLBAR_SIZE;
    if (HorzScroll_.getVisible() && Rect.Bottom > Rect_.Bottom - SCROLLBAR_SIZE)
        Rect.Bottom = Rect_.Bottom - SCROLLBAR_SIZE;
    
    bool isMouseOverExpand = false;
    
    Pos.Y += TREEITEM_HEIGHT;
    
    /* Update children */
    if (!Item->Children_.empty() || Item->hasExplorerSubDir_)
    {
        updateExpandIcon(dim::point2di(Pos.X - 8, Pos.Y - 9), Item, isMouseOverExpand);
        
        if (Item->getExpand())
        {
            Pos.X += TREEITEM_EXPAND_SIZE;
            
            foreach (GUITreeItem* Item, Item->Children_)
            {
                if (updateItem(Item, Pos))
                    return true;
            }
            
            Pos.X -= TREEITEM_EXPAND_SIZE;
        }
    }
    
    /* Update current item */
    if (!isMouseOverExpand && mouseOver(Rect))
    {
        Item->isPicked_ = true;
        
        if (mouseLeft())
        {
            SelectedItem_ = Item;
            
            SGUIEvent Event;
            {
                Event.Object    = EVENT_GADGET;
                Event.Type      = EVENT_ACTIVATE;
                Event.Gadget    = this;
                Event.SubData   = Item;
            }
            sendEvent(Event);
        }
    }
    
    return false;
}

bool GUITreeGadget::updateExpandIcon(const dim::point2di &Pos, GUITreeItem* Item, bool &isMouseOver)
{
    const dim::rect2di Rect(
        Pos.X - EXPANDICON_HALFSIZE - 1, Pos.Y - EXPANDICON_HALFSIZE,
        Pos.X + EXPANDICON_HALFSIZE, Pos.Y + EXPANDICON_HALFSIZE + 1
    );
    
    isMouseOver = mouseOver(Rect);
    
    if ( Item && ( !Item->Children_.empty() || Item->hasExplorerSubDir_ ) )
    {
        if (isMouseOver && mouseLeft())
        {
            Item->setExpand(!Item->getExpand());
            
            if (Item->getExpand() && Item->Children_.empty())
                return createExplorerDirs(Item);
        }
    }
    
    return false;
}

dim::point2di GUITreeGadget::getItemsStartPos() const
{
    dim::point2di ScrollPos(getScrollPosition());
    return dim::point2di(Rect_.Left + 2 + TREEITEM_EXPAND_SIZE + ScrollPos.X, Rect_.Top + 2 + ScrollPos.Y);
}

bool GUITreeGadget::createExplorerDirs(GUITreeItem* Item)
{
    #if defined(SP_PLATFORM_WINDOWS)
    
    bool Result = false;
    
    const io::stringc DirPath = getExplorerFullPath(Item) + "/*";
    
    WIN32_FIND_DATA FindFileData;
    HANDLE SearchHandle = FindFirstFile(DirPath.c_str(), &FindFileData);
    
    if (SearchHandle != INVALID_HANDLE_VALUE)
    {
        do
        {
            if (addExplorerSubItem(Item, (const void*)&FindFileData))
                Result = true;
        }
        while (FindNextFile(SearchHandle, &FindFileData));
        
        FindClose(SearchHandle);
    }
    
    return Result;
    
    #else
    
    return false;
    
    #endif
}

bool GUITreeGadget::addExplorerSubItem(GUITreeItem* Item, const void* FindFileDataRaw)
{
    #if defined(SP_PLATFORM_WINDOWS)
    
    if (!FindFileDataRaw)
        return false;
    
    const WIN32_FIND_DATA* FindFileData = (const WIN32_FIND_DATA*)FindFileDataRaw;
    
    /* Check if the entry is allowed */
    const io::stringc EntryName = FindFileData->cFileName;
    
    if (!(FindFileData->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ||
        FindFileData->dwFileAttributes & FILE_ATTRIBUTE_HIDDEN ||
        EntryName == "." || EntryName == "..")
    {
        return false;
    }
    
    /* Add the new sub item */
    GUITreeItem* SubItem = addItem(EntryName, Item);
    
    /* Check if the new sub item also has children directories */
    const io::stringc SubDirPath = getExplorerFullPath(SubItem) + "/*";
    
    WIN32_FIND_DATA SubFindFileData;
    HANDLE SearchSubHandle = FindFirstFile(SubDirPath.c_str(), &SubFindFileData);
    
    if (SearchSubHandle != INVALID_HANDLE_VALUE)
    {
        do
        {
            const io::stringc SubEntryName = SubFindFileData.cFileName;
            
            if (SubEntryName != "." && SubEntryName != ".." &&
                (SubFindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) &&
                !(SubFindFileData.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN))
            {
                SubItem->hasExplorerSubDir_ = true;
                break;
            }
        }
        while (FindNextFile(SearchSubHandle, &SubFindFileData));
        
        FindClose(SearchSubHandle);
    }
    
    #endif
    
    return true;
}


} // /namespace gui

} // /namespace sp


#endif







// ================================================================================
