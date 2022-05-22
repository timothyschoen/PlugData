struct TextObject : public ObjectBase, private TextEditor::Listener
{
    TextObject(void* obj, Box* parent) : ObjectBase(obj, parent)
    {
        currentText = getText();
    }

    void resized() override
    {
        auto* textObj = static_cast<t_text*>(ptr);
        textObj->te_width = getWidth();

        if (editor)
        {
            editor->setBounds(getLocalBounds());
        }
    }

    void paint(Graphics& g) override
    {
        g.setColour(findColour(ResizableWindow::backgroundColourId));
        g.fillRect(getLocalBounds().toFloat().reduced(0.5f));

        g.setColour(findColour(PlugDataColour::textColourId));
        g.setFont(font);

        auto textArea = border.subtractedFrom(getLocalBounds());

        g.drawFittedText(currentText, textArea, justification, jmax(1, static_cast<int>((static_cast<float>(textArea.getHeight()) / font.getHeight()))), minimumHorizontalScale);
    }

    void updateValue() override{};

    Type getType() override
    {
        return Type::Text;
    }

    void lock(bool isLocked) override
    {
        setInterceptsMouseClicks(!isLocked, !isLocked);
    }

    void mouseDown(const MouseEvent& e) override
    {
        wasSelected = cnv->isSelected(box);
        box->mouseDown(e.getEventRelativeTo(box));
    }

    void mouseDrag(const MouseEvent& e) override
    {
        box->mouseDrag(e.getEventRelativeTo(box));
    }

    void mouseUp(const MouseEvent& e) override
    {
        if (box->isEnabled() && !(e.mouseWasDraggedSinceMouseDown() || e.mods.isPopupMenu()) && wasSelected)
        {
            showEditor();
        }

        box->mouseUp(e.getEventRelativeTo(box));
    }

    int getBestTextWidth(const String& text)
    {
        return std::max<float>(round(font.getStringWidthFloat(text) + 30.5f), 40);
    }

    /*
    void setEditable(bool editable)
    {
        editSingleClick = editable;

        setWantsKeyboardFocus(editSingleClick);
        setFocusContainerType(editSingleClick ? FocusContainerType::keyboardFocusContainer : FocusContainerType::none);
        invalidateAccessibilityHandler();
    } */

    void textEditorReturnKeyPressed(TextEditor& ed) override
    {
        if (editor != nullptr)
        {
            editor->giveAwayKeyboardFocus();
        }
    }

    void textEditorTextChanged(TextEditor& ed) override
    {
        // For resize-while-typing behaviour
        auto width = getBestTextWidth(ed.getText());

        if (width > getWidth())
        {
            setSize(width, getHeight());
        }
    }

    void hideEditor() override
    {
        if (editor != nullptr)
        {
            WeakReference<Component> deletionChecker(this);
            std::unique_ptr<TextEditor> outgoingEditor;
            std::swap(outgoingEditor, editor);

            if (auto* peer = getPeer()) peer->dismissPendingTextInput();

            outgoingEditor->setInputFilter(nullptr, false);

            cnv->hideSuggestions();

            auto newText = outgoingEditor->getText();

            bool changed;
            if (currentText != newText)
            {
                currentText = newText;
                repaint();
                changed = true;
            }
            else
            {
                changed = false;
            }

            outgoingEditor.reset();

            repaint();

            // update if the name has changed, or if pdobject is unassigned
            if (changed)
            {
                box->setType(newText);
            }
        }
    }

    void updateBounds() override
    {
        int x, y, w, h;
        // If it's a text object, we need to handle the resizable width, which pd saves in amount of text characters
        auto* textObj = static_cast<t_text*>(ptr);

        libpd_get_object_bounds(cnv->patch.getPointer(), ptr, &x, &y, &w, &h);

        Rectangle<int> bounds = {x, y, textObj->te_width, h};

        int fontWidth = glist_fontwidth(cnv->patch.getPointer());
        int textWidth = getBestTextWidth(currentText);

        int mod = textWidth % fontWidth;

        textObjectWidth = bounds.getWidth();

        int width = textObjectWidth == 0 ? textWidth : (textObjectWidth * glist_fontwidth(cnv->patch.getPointer())) + textWidthOffset + Box::doubleMargin;

        box->setBounds(bounds.getX(), bounds.getY(), width, Box::height);
    }

    void showEditor() override
    {
        if (editor == nullptr)
        {
            editor = std::make_unique<TextEditor>(getName());
            editor->applyFontToAllText(font);

            copyAllExplicitColoursTo(*editor);
            editor->setColour(Label::textWhenEditingColourId, findColour(TextEditor::textColourId));
            editor->setColour(Label::backgroundWhenEditingColourId, findColour(TextEditor::backgroundColourId));
            editor->setColour(Label::outlineWhenEditingColourId, findColour(TextEditor::focusedOutlineColourId));

            editor->setAlwaysOnTop(true);

            editor->setMultiLine(false);
            editor->setReturnKeyStartsNewLine(false);
            editor->setBorder(border);
            editor->setIndents(0, 0);
            editor->setJustification(justification);

            editor->onFocusLost = [this]()
            {
                // Necessary so the editor doesn't close when clicking on a suggestion
                if (!reinterpret_cast<Component*>(cnv->suggestor)->hasKeyboardFocus(true))
                {
                    hideEditor();
                }
            };

            cnv->showSuggestions(box, editor.get());

            editor->setSize(10, 10);
            addAndMakeVisible(editor.get());

            editor->setText(currentText, false);
            editor->addListener(this);

            if (editor == nullptr)  // may be deleted by a callback
                return;

            editor->setHighlightedRegion(Range<int>(0, currentText.length()));

            resized();
            repaint();

            editor->grabKeyboardFocus();
        }
    }

    /** Returns the currently-visible text editor, or nullptr if none is open. */
    TextEditor* getCurrentTextEditor() const noexcept
    {
        return editor.get();
    }

   protected:
    Justification justification = Justification::centredLeft;
    std::unique_ptr<TextEditor> editor;
    BorderSize<int> border{1, 7, 1, 2};
    float minimumHorizontalScale = 1.0f;

    String currentText;
    Font font{15.0f};

    bool wasSelected = false;

    int textObjectWidth = 0;
    int textWidthOffset = 0;
};