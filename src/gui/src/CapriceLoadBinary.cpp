// 'Load Binary' window for Caprice32

#include "CapriceLoadBinary.h"
#include "cap32.h"
#include "wg_messagebox.h"
#include <cstdlib>
#include <string>

namespace wGui {
CapriceLoadBinary::CapriceLoadBinary(const CRect& WindowRect, CWindow* pParent, CFontEngine* pFontEngine)
    : CFrame(WindowRect, pParent, pFontEngine, "Load Binary", false) {
  SetModal(true);

  m_pFileLabel = new CLabel(CPoint(15, 25), this, "File:");
  m_pFilePath = new CEditBox(CRect(CPoint(60, 20), 200, 20), this);
  m_pFilePath->SetIsFocusable(true);

  m_pOffsetLabel = new CLabel(CPoint(15, 55), this, "Offset:");
  m_pOffset = new CEditBox(CRect(CPoint(60, 50), 100, 20), this);
  m_pOffset->SetIsFocusable(true);
  m_pOffset->SetContentType(CEditBox::HEXNUMBER);

  m_pCancelButton = new CButton(CRect(CPoint(60, 90), 80, 20), this, "Cancel");
  m_pCancelButton->SetIsFocusable(true);
  m_pLoadButton = new CButton(CRect(CPoint(160, 90), 80, 20), this, "Load");
  m_pLoadButton->SetIsFocusable(true);
}

CapriceLoadBinary::~CapriceLoadBinary() = default;

bool CapriceLoadBinary::HandleMessage(CMessage* pMessage) {
  bool bHandled = false;
  if (pMessage) {
    switch (pMessage->MessageType()) {
      case CMessage::CTRL_SINGLELCLICK: {
        if (pMessage->Destination() == this) {
          if (pMessage->Source() == m_pCancelButton) {
            CloseFrame();
            bHandled = true;
            break;
          }
          if (pMessage->Source() == m_pLoadButton) {
            std::string filename = m_pFilePath->GetWindowText();
            std::string offset_str = m_pOffset->GetWindowText();
            if (filename.empty() || offset_str.empty()) {
              auto* pMessageBox = new wGui::CMessageBox(
                  CRect(CPoint(m_ClientRect.Width() / 2 - 125, m_ClientRect.Height() / 2 - 30), 250, 60),
                  this, nullptr, "Error", "File or offset missing", CMessageBox::BUTTON_OK);
              pMessageBox->SetModal(true);
            } else {
              size_t offset = strtoul(offset_str.c_str(), nullptr, 16);
              bin_load(filename, offset, true);
              CloseFrame();
            }
            bHandled = true;
            break;
          }
        }
        break;
      }
      default:
        break;
    }
  }
  if (!bHandled) {
    bHandled = CFrame::HandleMessage(pMessage);
  }
  return bHandled;
}

} // namespace wGui

