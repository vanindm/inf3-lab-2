#include <wx/listbox.h>
#include <wx/splitter.h>
#include <wx/treectrl.h>
#include <wx/wx.h>

#include <wx/artprov.h>
#include <wx/aui/aui.h>
#include <wx/ribbon/bar.h>
#include <wx/ribbon/buttonbar.h>

#include <memory>
#include <string>

#include "Filemap.h"
#include "Filesystem.h"

class LabFSApp : public wxApp {
  public:
    virtual bool OnInit();
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
    wxSplitterWindow *detailsSplitter;

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

  public:
    MainFrame();
    void onRibbonButtonClicked(wxRibbonButtonBarEvent &event);
    wxDECLARE_EVENT_TABLE();
};

wxIMPLEMENT_APP(LabFSApp);

wxBEGIN_EVENT_TABLE(MainFrame, wxFrame)
    EVT_RIBBONBUTTONBAR_CLICKED(wxID_ADD, MainFrame::onRibbonButtonClicked)
    EVT_RIBBONBUTTONBAR_CLICKED(wxID_DELETE, MainFrame::onRibbonButtonClicked)
    EVT_RIBBONBUTTONBAR_CLICKED(wxID_HARDDISK, MainFrame::onRibbonButtonClicked)
wxEND_EVENT_TABLE()

                    bool LabFSApp::OnInit() {
    MainFrame *frame = new MainFrame();
    frame->Show(true);
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

void MainFrame::onRibbonButtonClicked(wxRibbonButtonBarEvent &event) {
    wxString message;
    wxFileDialog openFileDialog(this, _("Выберите файл для добавления"));
    wxTextEntryDialog newDirDialog(this, _("Введите название новой директории"));
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
    }
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
        mainRibbonPage, wxID_ANY, wxT("Удаление"), wxNullBitmap,
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

    ribbonBar->AddPageHighlight(ribbonBar->GetPageCount() - 1);
    ribbonBar->Realise();

    ribbonBar->DismissExpandedPanel();
    // ribbonBar->SetArtProvider(new wxRibbonMSWArtProvider);

    mainSplitter = new wxSplitterWindow(this);
    detailsSplitter = new wxSplitterWindow(mainSplitter);

    mainSplitter->SetSashGravity(0.3);
    detailsSplitter->SetSashGravity(0.5);

    fsTree = new wxTreeCtrl(mainSplitter, wxID_ANY);
    mainSplitter->SplitVertically(fsTree, detailsSplitter);

    wxSizer *sizer = new wxBoxSizer(wxVERTICAL);
    sizer->Add(ribbonBar, 0, wxEXPAND);
    sizer->Add(mainSplitter, 1, wxEXPAND);
    SetSizer(sizer);
    Center();

    buildFSTree();
}