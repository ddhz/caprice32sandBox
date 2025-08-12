// 'Load Binary' window for Caprice32

#include "CapriceLoadBinary.h"
#include "cap32.h"
#include "wg_messagebox.h"
#include "stringutils.h"
#include "log.h"

#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <dirent.h>
#include <errno.h>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <vector>
#ifdef WINDOWS
#include <filesystem>

char* realpath(const char* path, char* resolved_path) {
  try {
    auto fs_path = std::filesystem::path(path);
    auto canonical = std::filesystem::canonical(fs_path);
    strncpy(resolved_path, canonical.string().c_str(), _MAX_PATH);
  } catch (...) {
    return nullptr;
  }
  return resolved_path;
}
#endif

namespace wGui {

CapriceLoadBinary::CapriceLoadBinary(const CRect& WindowRect, CWindow* pParent, CFontEngine* pFontEngine)
    : CFrame(WindowRect, pParent, pFontEngine, "Load Binary", false) {
  SetModal(true);

  m_pDirectoryLabel = new CLabel(CPoint(15, 25), this, "Directory:");
  m_pDirectory = new CEditBox(CRect(CPoint(80, 20), 200, 20), this);
  m_pDirectory->SetReadOnly(true);
  m_pDirectory->SetWindowText(simplifyDirPath("."));

  m_pFilesList = new CListBox(CRect(CPoint(80, 50), 170, 80), this, true);
  m_pFilesList->SetIsFocusable(true);

  m_pFileLabel = new CLabel(CPoint(15, 145), this, "File:");
  m_pFilePath = new CEditBox(CRect(CPoint(80, 140), 200, 20), this);
  m_pFilePath->SetIsFocusable(true);
  m_pFilePath->SetReadOnly(true);

  m_pOffsetLabel = new CLabel(CPoint(15, 175), this, "Offset:");
  m_pOffset = new CEditBox(CRect(CPoint(80, 170), 100, 20), this);
  m_pOffset->SetIsFocusable(true);
  m_pOffset->SetContentType(CEditBox::HEXNUMBER);

  m_pCancelButton = new CButton(CRect(CPoint(80, 205), 80, 20), this, "Cancel");
  m_pCancelButton->SetIsFocusable(true);
  m_pLoadButton = new CButton(CRect(CPoint(180, 205), 80, 20), this, "Load");
  m_pLoadButton->SetIsFocusable(true);

  UpdateFilesList();
}

CapriceLoadBinary::~CapriceLoadBinary() = default;

std::string CapriceLoadBinary::simplifyDirPath(std::string path) {
#ifdef WINDOWS
  char simplepath[_MAX_PATH + 1];
#else
  char simplepath[PATH_MAX + 1];
#endif
  if (realpath(path.c_str(), simplepath) == nullptr) {
    LOG_ERROR("Couldn't simplify path '" << path << "': " << strerror(errno));
    return ".";
  }
  struct stat entry_infos;
  if (stat(simplepath, &entry_infos) != 0) {
    LOG_ERROR("Could not retrieve info on " << simplepath << ": " << strerror(errno));
    return ".";
  }
  if (!S_ISDIR(entry_infos.st_mode)) {
    LOG_ERROR(simplepath << " is not a directory.");
    return ".";
  }
  return std::string(simplepath);
}

void CapriceLoadBinary::UpdateFilesList() {
  m_pFilesList->ClearItems();

  DIR* dp;
  struct dirent* ep;

  dp = opendir(m_pDirectory->GetWindowText().c_str());
  if (dp == nullptr) {
    LOG_ERROR("Could not open " << m_pDirectory->GetWindowText() << ": " << strerror(errno));
  } else {
    std::vector<std::string> directories;
    std::vector<std::string> files;
    while ((ep = readdir(dp)) != nullptr) {
      std::string entry_name = ep->d_name;
      struct stat entry_infos;
      std::string full_name = m_pDirectory->GetWindowText() + "/" + entry_name;
      if (stat(full_name.c_str(), &entry_infos) != 0) {
        LOG_ERROR("Could not retrieve info on " << full_name << ": " << strerror(errno));
      }
      if (S_ISDIR(entry_infos.st_mode) && (ep->d_name[0] != '.' || entry_name == "..")) {
        directories.push_back(entry_name + "/");
      } else if (S_ISREG(entry_infos.st_mode)) {
        files.push_back(entry_name);
      }
    }
    if (closedir(dp) != 0) {
      LOG_ERROR("Could not close directory: " << strerror(errno));
    }
    std::sort(directories.begin(), directories.end(), stringutils::caseInsensitiveCompare);
    std::sort(files.begin(), files.end(), stringutils::caseInsensitiveCompare);
    for (const auto& directory : directories) {
      m_pFilesList->AddItem(SListItem(directory));
    }
    for (const auto& file : files) {
      m_pFilesList->AddItem(SListItem(file));
    }
  }
}

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
          if (pMessage->Source() == m_pFilesList) {
            int idx = m_pFilesList->getFirstSelectedIndex();
            std::string fn;
            if (idx != -1) {
              fn = m_pFilesList->GetItem(idx).sItemText;
            }
            if (!fn.empty() && fn.back() == '/') {
              m_pDirectory->SetWindowText(
                  simplifyDirPath(m_pDirectory->GetWindowText() + '/' + fn));
              m_pFilePath->SetWindowText("");
              UpdateFilesList();
            } else if (!fn.empty()) {
              m_pFilePath->SetWindowText(m_pDirectory->GetWindowText() + '/' + fn);
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

