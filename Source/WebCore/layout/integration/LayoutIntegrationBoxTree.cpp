/*
 * Copyright (C) 2020 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"
#include "LayoutIntegrationBoxTree.h"

#if ENABLE(LAYOUT_FORMATTING_CONTEXT)

#include "LayoutInlineTextBox.h"
#include "LayoutLineBreakBox.h"
#include "LayoutReplacedBox.h"
#include "RenderBlockFlow.h"
#include "RenderChildIterator.h"
#include "RenderImage.h"
#include "RenderLineBreak.h"

namespace WebCore {
namespace LayoutIntegration {

static constexpr size_t smallTreeThreshold = 8;

static RenderStyle rootBoxStyle(const RenderStyle& style)
{
    auto clonedStyle = RenderStyle::clone(style);
    clonedStyle.setDisplay(DisplayType::Block);
    return clonedStyle;
}

BoxTree::BoxTree(RenderBlockFlow& flow)
    : m_flow(flow)
    , m_root(rootBoxStyle(flow.style()))
{
    if (flow.isAnonymous())
        m_root.setIsAnonymous();

    buildTree();
}

void BoxTree::buildTree()
{
    for (auto& childRenderer : childrenOfType<RenderObject>(m_flow)) {
        std::unique_ptr<Layout::Box> childBox;
        if (is<RenderText>(childRenderer)) {
            auto& textRenderer = downcast<RenderText>(childRenderer);
            childBox = makeUnique<Layout::InlineTextBox>(textRenderer.text(), textRenderer.canUseSimplifiedTextMeasuring(), RenderStyle::createAnonymousStyleWithDisplay(m_root.style(), DisplayType::Inline));
        } else if (childRenderer.isLineBreak()) {
            auto clonedStyle = RenderStyle::clone(childRenderer.style());
            clonedStyle.setDisplay(DisplayType::Inline);
            clonedStyle.setFloating(Float::No);
            clonedStyle.setPosition(PositionType::Static);
            childBox = makeUnique<Layout::LineBreakBox>(downcast<RenderLineBreak>(childRenderer).isWBR(), WTFMove(clonedStyle));
        } else if (is<RenderReplaced>(childRenderer)) {
            auto clonedStyle = RenderStyle::clone(childRenderer.style());
            childBox = makeUnique<Layout::ReplacedBox>(Layout::Box::ElementAttributes { is<RenderImage>(childRenderer) ? Layout::Box::ElementType::Image : Layout::Box::ElementType::GenericElement }, WTFMove(clonedStyle));
        } else if (is<RenderBlock>(childRenderer)) {
            auto clonedStyle = RenderStyle::clone(childRenderer.style());
            childBox = makeUnique<Layout::ReplacedBox>(Layout::Box::ElementAttributes { Layout::Box::ElementType::GenericElement }, WTFMove(clonedStyle));
        }
        ASSERT(childBox);

        m_root.appendChild(*childBox);
        m_boxes.append({ WTFMove(childBox), &childRenderer });
    }
}

void BoxTree::updateStyle(const RenderBoxModelObject& renderer)
{
    auto& layoutBox = layoutBoxForRenderer(renderer);
    auto& style = renderer.style();

    layoutBox.updateStyle(style);

    if (&layoutBox == &m_root) {
        for (auto* child = m_root.firstChild(); child; child = child->nextSibling()) {
            if (child->isAnonymous())
                child->updateStyle(RenderStyle::createAnonymousStyleWithDisplay(style, DisplayType::Inline));
        }
    }
}

Layout::Box& BoxTree::layoutBoxForRenderer(const RenderObject& renderer)
{
    if (&renderer == &m_flow)
        return m_root;

    if (m_boxes.size() <= smallTreeThreshold) {
        auto index = m_boxes.findMatching([&](auto& entry) {
            return entry.renderer == &renderer;
        });
        ASSERT(index != notFound);
        return *m_boxes[index].box;
    }

    if (m_rendererToBoxMap.isEmpty()) {
        for (auto& entry : m_boxes)
            m_rendererToBoxMap.add(entry.renderer, entry.box.get());
    }
    return *m_rendererToBoxMap.get(&renderer);
}

const Layout::Box& BoxTree::layoutBoxForRenderer(const RenderObject& renderer) const
{
    return const_cast<BoxTree&>(*this).layoutBoxForRenderer(renderer);
}

RenderObject& BoxTree::rendererForLayoutBox(const Layout::Box& box)
{
    if (&box == &m_root)
        return m_flow;

    if (m_boxes.size() <= smallTreeThreshold) {
        auto index = m_boxes.findMatching([&](auto& entry) {
            return entry.box.get() == &box;
        });
        ASSERT(index != notFound);
        return *m_boxes[index].renderer;
    }

    if (m_boxToRendererMap.isEmpty()) {
        for (auto& entry : m_boxes)
            m_boxToRendererMap.add(entry.box.get(), entry.renderer);
    }
    return *m_boxToRendererMap.get(&box);
}

const RenderObject& BoxTree::rendererForLayoutBox(const Layout::Box& box) const
{
    return const_cast<BoxTree&>(*this).rendererForLayoutBox(box);
}

}
}

#endif


