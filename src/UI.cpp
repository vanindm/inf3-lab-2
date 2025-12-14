#include <wx/listbox.h>
#include <wx/splitter.h>
#include <wx/treectrl.h>
#include <wx/wx.h>

#include <wx/artprov.h>
#include <wx/aui/aui.h>
#include <wx/ribbon/bar.h>
#include <wx/ribbon/buttonbar.h>

#include <format>
#include <memory>
#include <string>

#include "Filemap.h"
#include "Filesystem.h"

class LabFSApp : public wxApp {
  public:
    virtual bool OnInit();
    virtual bool OnExceptionInMainLoop() override;
};

class MainFrame : public wxFrame {
    wxRibbonBar *ribbonBar;

    wxRibbonPage *mainRibbonPage;
    wxRibbonPage *permissionsRibbonPage;

    wxRibbonPanel *addPanel;
    wxRibbonButtonBar *addButtonBar;
    wxRibbonPanel *deletePanel;
    wxRibbonButtonBar *deleteButtonBar;

    wxSplitterWindow *mainSplitter;
    wxTextCtrl *descriptionText;

    wxTreeCtrl *fsTree;
    LabFS::Filesystem fs;

    class FSTreeItemData : public wxTreeItemData {
        std::shared_ptr<LabFS::Filesystem::Node> fsNode;

      public:
        FSTreeItemData(std::shared_ptr<LabFS::Filesystem::Node> fsNode)
            : fsNode(fsNode) {}
        std::shared_ptr<LabFS::Filesystem::Node> getFSNode() const {
            return fsNode;
        }
    };

    void buildFSTree();
    void addFile(const wxString &filename, const LabFS::Path &path);
    void addDirectory(const wxString &name);
    void deleteNode();
    void moveNode(LabFS::Path &path);
    void updateDescriptionText();

  public:
    MainFrame();
    void onRibbonButtonClicked(wxRibbonButtonBarEvent &event);
    void onFSTreeSelectionChanged(wxTreeEvent &event);
    wxDECLARE_EVENT_TABLE();
};

wxIMPLEMENT_APP(LabFSApp);

wxBEGIN_EVENT_TABLE(MainFrame, wxFrame)
    EVT_RIBBONBUTTONBAR_CLICKED(wxID_ADD, MainFrame::onRibbonButtonClicked)
        EVT_RIBBONBUTTONBAR_CLICKED(wxID_DELETE,
                                    MainFrame::onRibbonButtonClicked)
            EVT_RIBBONBUTTONBAR_CLICKED(wxID_HARDDISK,
                                        MainFrame::onRibbonButtonClicked)
                EVT_RIBBONBUTTONBAR_CLICKED(wxID_EDIT,
                                            MainFrame::onRibbonButtonClicked)
                    EVT_TREE_SEL_CHANGED(wxID_TOP,
                                         MainFrame::onFSTreeSelectionChanged)
                        wxEND_EVENT_TABLE()

                            bool LabFSApp::OnInit() {
    MainFrame *frame = new MainFrame();
    frame->Show(true);
    return true;
}

bool LabFSApp::OnExceptionInMainLoop() {
    try {
        throw;
    } catch (const std::exception &exception) {
        wxMessageBox(wxString::FromUTF8(exception.what()),
                     wxString::FromUTF8("Ошибка"), wxOK | wxICON_ERROR);
    }
    return true;
}

void MainFrame::buildFSTree() {
    auto root = fsTree->AddRoot(fs.getRootDirectory()->GetName());
    FSTreeItemData *ptr = new FSTreeItemData(fs.getRootDirectory());
    fsTree->SetItemData(root, (wxTreeItemData *)(ptr));
}

void MainFrame::addFile(const wxString &filename, const LabFS::Path &path) {
    wxTreeItemId currentNodeID = fsTree->GetSelection();
    if (((FSTreeItemData *)fsTree->GetItemData(currentNodeID))
            ->getFSNode()
            ->GetType() == LabFS::Filesystem::NODE_FILE) {
        currentNodeID = fsTree->GetItemParent(currentNodeID);
    }
    FSTreeItemData *currentNodeInfo =
        (FSTreeItemData *)fsTree->GetItemData(currentNodeID);
    if (currentNodeInfo->getFSNode() != nullptr) {
        auto newFSNode = currentNodeInfo->getFSNode()->AddFileSubnode(
            fs.addFile(filename.ToStdString(), path));
        wxTreeItemId newNode = fsTree->AppendItem(currentNodeID, filename);
        fsTree->SetItemData(newNode,
                            (wxTreeItemData *)(new FSTreeItemData(newFSNode)));
    } else {
        throw std::logic_error("попытка получить нулевой указатель");
    }
}

void MainFrame::addDirectory(const wxString &name) {
    wxTreeItemId currentNodeID = fsTree->GetSelection();
    if (((FSTreeItemData *)fsTree->GetItemData(currentNodeID))
            ->getFSNode()
            ->GetType() == LabFS::Filesystem::NODE_FILE) {
        currentNodeID = fsTree->GetItemParent(currentNodeID);
    }
    FSTreeItemData *currentNodeInfo =
        (FSTreeItemData *)fsTree->GetItemData(currentNodeID);

    auto newFSNode =
        currentNodeInfo->getFSNode()->AddDirectorySubnode(name.ToStdString());
    wxTreeItemId newNode = fsTree->AppendItem(currentNodeID, name);
    fsTree->SetItemData(newNode,
                        (wxTreeItemData *)(new FSTreeItemData(newFSNode)));
}

void MainFrame::deleteNode() {
    wxTreeItemId currentNodeID = fsTree->GetSelection();
    if (currentNodeID == fsTree->GetRootItem()) {
        wxMessageDialog cannotDeleteRootDialog(
            GetParent(), wxT("Невозможно удалить корень!"), wxT("Ошибка"));
        cannotDeleteRootDialog.ShowModal();
        return;
    }
    fsTree->Delete(currentNodeID);
}

void MainFrame::moveNode(LabFS::Path &toWhere) {
    wxTreeItemId currentNodeID = fsTree->GetSelection();
    if (currentNodeID == fsTree->GetRootItem()) {
        wxMessageDialog cannotMoveRootDialog(
            GetParent(), wxT("Невозможно переместить корень!"), wxT("Ошибка"));
        cannotMoveRootDialog.ShowModal();
        return;
    }
    auto enumeratePath = toWhere.getEnumerator();
    auto foundNode = wxTreeItemId();
    auto currentNode = fsTree->GetRootItem();
    if (toWhere.toString() != "/") {
        while (enumeratePath->moveNext()) {
            wxTreeItemIdValue cookie;
            wxTreeItemId item = fsTree->GetFirstChild(currentNode, cookie);
            while (item.IsOk()) {
                wxString sData = fsTree->GetItemText(item);
                if (sData.ToStdString() == enumeratePath->current()) {
                    break;
                }
            }
            wxString sData = fsTree->GetItemText(item);
            if (sData.ToStdString() == enumeratePath->current()) {
                break;
            } else {
                item = fsTree->GetNextChild(item, cookie);
            }
        }
        if (!foundNode.IsOk()) {
            throw std::logic_error(
                "Не найдена директория, в которую следует переместить!");
        }
    } else {
        foundNode = fsTree->GetRootItem();
    }
    auto foundItemData = (FSTreeItemData *)(fsTree->GetItemData(foundNode));
    if (foundItemData->getFSNode()->GetType() == LabFS::Filesystem::NODE_FILE) {
        throw std::logic_error("попытка переместить не в директорию!");
    }
    ((FSTreeItemData *)fsTree->GetItemData(currentNodeID))
        ->getFSNode()
        ->move(foundItemData->getFSNode());
    auto newNode = fsTree->AppendItem(
        foundNode, ((FSTreeItemData *)fsTree->GetItemData(currentNodeID))
                       ->getFSNode()
                       ->GetName());
    fsTree->SetItemData(
        newNode, (wxTreeItemData *)(new FSTreeItemData(
                                        ((FSTreeItemData *)fsTree->GetItemData(
                                            currentNodeID))->getFSNode())));
    fsTree->Delete(currentNodeID);
}

void MainFrame::onRibbonButtonClicked(wxRibbonButtonBarEvent &event) {
    wxString message;
    wxFileDialog openFileDialog(this, wxT("Выберите файл для добавления"));
    wxTextEntryDialog newDirDialog(this,
                                   wxT("Введите название новой директории"));
    wxTextEntryDialog moveDialog(this, wxT("Куда переместить?"));
    switch (event.GetBar()->GetItemId(event.GetButton())) {
    case wxID_ADD:
        if (openFileDialog.ShowModal() == wxID_CANCEL)
            return;
        addFile(openFileDialog.GetFilename(),
                openFileDialog.GetPath().ToStdString());
        break;
    case wxID_DELETE:
        deleteNode();
        break;
    case wxID_HARDDISK:
        if (newDirDialog.ShowModal() == wxID_CANCEL) {
            return;
        }
        addDirectory(newDirDialog.GetValue());
        break;
    case wxID_EDIT:
        if (moveDialog.ShowModal() == wxID_CANCEL) {
            return;
        }
        auto path = LabFS::Path(moveDialog.GetValue().ToStdString());
        moveNode(path);
        break;
    }
}

void MainFrame::onFSTreeSelectionChanged(wxTreeEvent &event) {
    try {
        updateDescriptionText();
    } catch (std::exception &e) {
        wxString s(e.what());
        wxMessageDialog errorBox(GetParent(), s, wxT("Ошибка"));
        errorBox.ShowModal();
    }
}

void MainFrame::updateDescriptionText() {
    wxTreeItemId currentNodeID = fsTree->GetSelection();
    FSTreeItemData *currentNodeInfo =
        (FSTreeItemData *)fsTree->GetItemData(currentNodeID);
    std::shared_ptr<LabFS::Filesystem::Node> node =
        currentNodeInfo->getFSNode();
    std::string description;
    if (currentNodeInfo->getFSNode()->GetType() ==
        LabFS::Filesystem::NODE_DIRECTORY) {
        description = std::format("Имя директории:\t{}",
                                  currentNodeInfo->getFSNode()->GetName());
    } else {
        description = std::format(
            "Имя файла:\t{}\nПуть до файла:\t{}\nХэш файла:\t{}\nРазмер "
            "файла:\t{}\n",
            currentNodeInfo->getFSNode()->GetFile()->getName(),
            currentNodeInfo->getFSNode()->GetFile()->getPath().toString(),
            currentNodeInfo->getFSNode()->GetFile()->getHash(),
            currentNodeInfo->getFSNode()->GetFile()->getSize());
    }
    descriptionText->SetValue(wxString::FromUTF8(description));
}

MainFrame::MainFrame()
    : wxFrame(NULL, wxID_ANY, "Файловая система LabFS", wxDefaultPosition,
              wxSize(800, 600)) {
    ribbonBar = new wxRibbonBar(
        this, -1, wxDefaultPosition, wxDefaultSize,
        wxRIBBON_BAR_FLOW_HORIZONTAL | wxRIBBON_BAR_SHOW_PAGE_LABELS |
            wxRIBBON_BAR_SHOW_PANEL_EXT_BUTTONS); //|
                                                  // wxRIBBON_BAR_SHOW_TOGGLE_BUTTON);

    mainRibbonPage =
        new wxRibbonPage(ribbonBar, wxID_ANY, wxT("Главное"), wxNullBitmap);
    permissionsRibbonPage =
        new wxRibbonPage(ribbonBar, wxID_ANY, wxT("Разрешения"), wxNullBitmap);

    addPanel = new wxRibbonPanel(mainRibbonPage, wxID_ANY, wxT("Добавление"),
                                 wxNullBitmap, wxDefaultPosition, wxDefaultSize,
                                 wxRIBBON_PANEL_NO_AUTO_MINIMISE);

    deletePanel = new wxRibbonPanel(
        mainRibbonPage, wxID_ANY, wxT("Изменение"), wxNullBitmap,
        wxDefaultPosition, wxDefaultSize, wxRIBBON_PANEL_NO_AUTO_MINIMISE);

    addButtonBar = new wxRibbonButtonBar(addPanel);
    deleteButtonBar = new wxRibbonButtonBar(deletePanel, -1, wxDefaultPosition,
                                            wxSize(64, 64));

    addButtonBar->AddButton(wxID_ADD, wxT("Добавить файл"),
                            wxArtProvider::GetBitmap(wxART_NORMAL_FILE,
                                                     wxART_TOOLBAR,
                                                     wxSize(32, 32)));
    addButtonBar->AddButton(
        wxID_HARDDISK, wxT("Добавить директорию"),
        wxArtProvider::GetBitmap(wxART_NEW_DIR, wxART_TOOLBAR, wxSize(32, 32)));
    deleteButtonBar->AddButton(
        wxID_DELETE, wxT("Удалить"),
        wxArtProvider::GetBitmap(wxART_DELETE, wxART_TOOLBAR, wxSize(32, 32)));
    deleteButtonBar->AddButton(
        wxID_EDIT, wxT("Переместить"),
        wxArtProvider::GetBitmap(wxART_EDIT, wxART_TOOLBAR, wxSize(32, 32)));

    ribbonBar->AddPageHighlight(ribbonBar->GetPageCount() - 1);
    ribbonBar->Realise();

    ribbonBar->DismissExpandedPanel();
    // ribbonBar->SetArtProvider(new wxRibbonMSWArtProvider);

    mainSplitter = new wxSplitterWindow(this);
    descriptionText = new wxTextCtrl(
        mainSplitter, wxID_ANY, "", wxDefaultPosition, wxDefaultSize,
        wxTE_MULTILINE | wxALIGN_TOP | wxTE_READONLY);

    mainSplitter->SetSashGravity(0.3);

    fsTree = new wxTreeCtrl(mainSplitter, wxID_TOP);

    mainSplitter->SplitVertically(fsTree, descriptionText);

    wxSizer *sizer = new wxBoxSizer(wxVERTICAL);
    sizer->Add(ribbonBar, 0, wxEXPAND);
    sizer->Add(mainSplitter, 1, wxEXPAND);
    SetSizer(sizer);
    Center();

    buildFSTree();
}