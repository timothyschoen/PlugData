
struct t_fake_textbuf
{
    t_object b_ob;
    t_binbuf *b_binbuf;
    t_canvas *b_canvas;
    t_guiconnect *b_guiconnect;
    t_symbol *b_sym;
};


struct t_fake_text_define
{
    t_fake_textbuf x_textbuf;
    t_outlet *x_out;
    t_outlet *x_notifyout;
    t_symbol *x_bindsym;
    t_scalar *x_scalar;     /* faux scalar (struct text-scalar) to point to */
    t_gpointer x_gp;        /* pointer to it */
    t_canvas *x_canvas;     /* owning canvas whose stub we use for x_gp */
    unsigned char x_keep;   /* whether to embed contents in patch on save */
};

struct TextEditorDialog : public Component
{
    
    TextEditorDialog() {
        addToDesktop(ComponentPeer::windowIsTemporary);
    }
};

// Actual text object, marked final for optimisation
struct TextDefineObject final : public TextBase
{
    
    TextDefineObject(void* obj, Box* parent, bool isValid = true) : TextBase(obj, parent, isValid)
    {
        
    }
    
    void setText(String text) {
        auto& textbuf = static_cast<t_fake_text_define*>(ptr)->x_textbuf;
        auto* binbuf = textbuf.b_binbuf;
        binbuf_clear(binbuf);
        binbuf_text(binbuf, text.toRawUTF8(), text.getNumBytesAsUTF8());
    }
    
    
    String getText() {
        auto& textbuf = static_cast<t_fake_text_define*>(ptr)->x_textbuf;
        auto* binbuf = textbuf.b_binbuf;
        
        char** bufp = new char*;
        int* lenp = new int;
        
        binbuf_gettext(binbuf, bufp, lenp);
        
        return String::fromUTF8(*bufp, *lenp);
        
    }
};