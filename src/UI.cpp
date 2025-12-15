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
#include "PermissionsDialog.h"

class LabFSApp : public wxApp {
  public:
    virtual bool OnInit();
    virtual bool OnExceptionInMainLoop() override;
};

LabFS::Filesystem::Subject* subject = LabFS::Filesystem::Subject::GetSubject(1);

class MainFrame : public wxFrame {
    wxRibbonBar *ribbonBar;

    wxRibbonPage *mainRibbonPage;
    wxRibbonPage *permissionsRibbonPage;

    wxRibbonPanel *addPanel;
    wxRibbonButtonBar *addButtonBar;
    wxRibbonPanel *deletePanel;
    wxRibbonButtonBar *deleteButtonBar;

    wxRibbonPanel *permissionsPanel;
    wxRibbonButtonBar *permissionsButtonBar;

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
    void moveNodeID(wxTreeItemId Node, wxTreeItemId newParent,
                    LabFS::Path &toWhere);
    void updateDescriptionText();
    void changeSubject();
    //void changeSubjectOf(LabFS::Path& path);
    void changePermsOf();

  public:
    MainFrame();
    void onRibbonButtonClicked(wxRibbonButtonBarEvent &event);
    void onFSTreeSelectionChanged(wxTreeEvent &event);
    wxDECLARE_EVENT_TABLE();
};

// clang-format off

wxIMPLEMENT_APP(LabFSApp);

wxBEGIN_EVENT_TABLE(MainFrame, wxFrame)
    EVT_RIBBONBUTTONBAR_CLICKED(wxID_ADD, MainFrame::onRibbonButtonClicked)
    EVT_RIBBONBUTTONBAR_CLICKED(wxID_DELETE, MainFrame::onRibbonButtonClicked)
    EVT_RIBBONBUTTONBAR_CLICKED(wxID_HARDDISK, MainFrame::onRibbonButtonClicked)
    EVT_RIBBONBUTTONBAR_CLICKED(wxID_EDIT, MainFrame::onRibbonButtonClicked)
    EVT_RIBBONBUTTONBAR_CLICKED(wxID_SETUP, MainFrame::onRibbonButtonClicked)
    EVT_RIBBONBUTTONBAR_CLICKED(wxID_CLEAR, MainFrame::onRibbonButtonClicked)
    EVT_TREE_SEL_CHANGED(wxID_TOP, MainFrame::onFSTreeSelectionChanged)
wxEND_EVENT_TABLE()

bool LabFSApp::OnInit() {
    MainFrame *frame = new MainFrame();
    frame->Show(true);
    return true;
}
// clang-format on

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
    auto rootDir = fs.getRootDirectory();
    FSTreeItemData *ptr = new FSTreeItemData(rootDir);
    fsTree->SetItemData(root, (wxTreeItemData *)(ptr));
    rootDir->SetReference(new wxTreeItemId(root));
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
            fs.addFile(filename.ToStdString(), path), subject);
        wxTreeItemId newNode = fsTree->AppendItem(currentNodeID, filename);
        fsTree->SetItemData(newNode,
                            (wxTreeItemData *)(new FSTreeItemData(newFSNode)));
        newFSNode->SetReference(new wxTreeItemId(newNode));
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
        currentNodeInfo->getFSNode()->AddDirectorySubnode(name.ToStdString(), subject);
    wxTreeItemId newNode = fsTree->AppendItem(currentNodeID, name);
    fsTree->SetItemData(newNode,
                        (wxTreeItemData *)(new FSTreeItemData(newFSNode)));
    newFSNode->SetReference(new wxTreeItemId(newNode));
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

void MainFrame::moveNodeID(wxTreeItemId Node, wxTreeItemId newParent,
                           LabFS::Path &toWhere) {
    wxTreeItemId currentNodeID = Node;
    if (currentNodeID == fsTree->GetRootItem()) {
        wxMessageDialog cannotMoveRootDialog(
            GetParent(), wxT("Невозможно переместить корень!"), wxT("Ошибка"));
        cannotMoveRootDialog.ShowModal();
        return;
    }

    auto fsNode =
        ((FSTreeItemData *)fsTree->GetItemData(currentNodeID))->getFSNode();

    auto newNode = fsTree->AppendItem(
        newParent, ((FSTreeItemData *)fsTree->GetItemData(currentNodeID))
                       ->getFSNode()
                       ->GetName());

    fsNode->SetReference(new wxTreeItemId(newNode));

    delete (
        wxTreeItemId *)((FSTreeItemData *)fsTree->GetItemData(currentNodeID))
        ->getFSNode()
        ->GetReference();
    ((FSTreeItemData *)fsTree->GetItemData(currentNodeID))
        ->getFSNode()
        ->SetReference(new wxTreeItemId(newNode));

    fsTree->SetItemData(
        newNode, (wxTreeItemData *)(new FSTreeItemData(
                     ((FSTreeItemData *)fsTree->GetItemData(currentNodeID))
                         ->getFSNode())));

    if (((FSTreeItemData *)fsTree->GetItemData(currentNodeID))
            ->getFSNode()
            ->GetType() == LabFS::Filesystem::NODE_DIRECTORY) {
        wxTreeItemIdValue cookie;
        wxTreeItemId item = fsTree->GetFirstChild(currentNodeID, cookie);
        while (item.IsOk()) {
            moveNodeID(item, newNode,
                       toWhere.add(((FSTreeItemData *)fsTree->GetItemData(
                                        currentNodeID))
                                       ->getFSNode()
                                       ->GetName()));
            item = fsTree->GetNextChild(item, cookie);
        }
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
    auto targetFSNode = fs.getNodeByPath(toWhere);
    wxTreeItemId *targetFSTreeNode =
        (wxTreeItemId *)targetFSNode->GetReference();
    if (targetFSTreeNode == nullptr) {
        throw std::logic_error("Директории не существует");
    }

    // auto foundItemData = (FSTreeItemData
    // *)(fsTree->GetItemData(*targetFSTreeNode));
    if (targetFSNode->GetType() == LabFS::Filesystem::NODE_FILE) {
        throw std::logic_error("попытка переместить не в директорию!");
    }

    ((FSTreeItemData *)fsTree->GetItemData(currentNodeID))
        ->getFSNode()
        ->move(targetFSNode, subject);

    auto newNode = fsTree->AppendItem(
        *targetFSTreeNode,
        ((FSTreeItemData *)fsTree->GetItemData(currentNodeID))
            ->getFSNode()
            ->GetName());

    delete (
        wxTreeItemId *)((FSTreeItemData *)fsTree->GetItemData(currentNodeID))
        ->getFSNode()
        ->GetReference();
    ((FSTreeItemData *)fsTree->GetItemData(currentNodeID))
        ->getFSNode()
        ->SetReference(new wxTreeItemId(newNode));

    fsTree->SetItemData(
        newNode, (wxTreeItemData *)(new FSTreeItemData(
                     ((FSTreeItemData *)fsTree->GetItemData(currentNodeID))
                         ->getFSNode())));

    if (((FSTreeItemData *)fsTree->GetItemData(currentNodeID))
            ->getFSNode()
            ->GetType() == LabFS::Filesystem::NODE_DIRECTORY) {
        wxTreeItemIdValue cookie;
        wxTreeItemId item = fsTree->GetFirstChild(currentNodeID, cookie);
        while (item.IsOk()) {
            moveNodeID(item, newNode,
                       toWhere.add(((FSTreeItemData *)fsTree->GetItemData(
                                        currentNodeID))
                                       ->getFSNode()
                                       ->GetName()));
            item = fsTree->GetNextChild(item, cookie);
        }
    }
    fsTree->Delete(currentNodeID);
}

void MainFrame::changeSubject() {
    wxTextEntryDialog subjectDialog(this, wxT("Введите желаемый ID пользователя"));
    if (subjectDialog.ShowModal() == wxID_CANCEL)
        return;
    unsigned int resSubject = 0;
    subjectDialog.GetValue().ToUInt(&resSubject);
    subject = LabFS::Filesystem::Subject::GetSubject(resSubject);
}

void MainFrame::changePermsOf() {
    PermissionsDialog permissionsDialog( wxT("Введите желаемые разрешения"));
    if (permissionsDialog.ShowModal() == wxID_CANCEL)
        return;
    LabFS::Filesystem::Permissions perms = permissionsDialog.GetValue();
    auto node = ((FSTreeItemData *)fsTree->GetItemData(fsTree->GetSelection()))
        ->getFSNode();
    node->SetPermissions(subject, perms);

}

void MainFrame::onRibbonButtonClicked(wxRibbonButtonBarEvent &event) {
    wxString message;
    wxFileDialog openFileDialog(this, wxT("Выберите файл для добавления"));
    wxTextEntryDialog newDirDialog(this,
                                   wxT("Введите название новой директории"));
    wxTextEntryDialog moveDialog(this, wxT("Куда переместить?"));
    LabFS::Path path("/");
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
        path = LabFS::Path(moveDialog.GetValue().ToStdString());
        moveNode(path);
        break;
    case wxID_SETUP:
        changeSubject();
        break;
    case wxID_CLEAR:
        changePermsOf();
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

    permissionsPanel = new wxRibbonPanel(permissionsRibbonPage, wxID_ANY,
                                         wxT("Разрешения"), wxNullBitmap);
    permissionsButtonBar = new wxRibbonButtonBar(permissionsPanel);
    permissionsButtonBar->AddButton(
        wxID_SETUP, wxT("Задать пользователя"),
        wxArtProvider::GetBitmap(wxART_REFRESH, wxART_TOOLBAR, wxSize(32, 32)));

    permissionsButtonBar->AddButton(
        wxID_CLEAR, wxT("Задать разрешения"),
        wxArtProvider::GetBitmap(wxART_FOLDER, wxART_TOOLBAR, wxSize(32, 32)));

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