// 'Load Binary' window for Caprice32

#ifndef _WG_CAPRICE32LOADBINARY_H_
#define _WG_CAPRICE32LOADBINARY_H_

#include "wg_button.h"
#include "wg_editbox.h"
#include "wg_frame.h"
#include "wg_label.h"

namespace wGui {
class CapriceLoadBinary : public CFrame {
 public:
  CapriceLoadBinary(const CRect& WindowRect, CWindow* pParent, CFontEngine* pFontEngine);
  ~CapriceLoadBinary() override;
  bool HandleMessage(CMessage* pMessage) override;

 private:
  CLabel* m_pFileLabel;
  CEditBox* m_pFilePath;
  CLabel* m_pOffsetLabel;
  CEditBox* m_pOffset;
  CButton* m_pLoadButton;
  CButton* m_pCancelButton;

  CapriceLoadBinary(const CapriceLoadBinary&) = delete;
  CapriceLoadBinary& operator=(const CapriceLoadBinary&) = delete;
};
}

#endif
