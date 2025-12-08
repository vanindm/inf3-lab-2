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
    wxRibbonPanel *fileManipPanel;
    wxRibbonButtonBar *addFileButtonBar;
    wxRibbonButtonBar *delFileButtonbar;

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

  public:
    MainFrame();
	void onRibbonButtonClicked(wxRibbonButtonBarEvent &event);
	wxDECLARE_EVENT_TABLE();
};

wxIMPLEMENT_APP(LabFSApp);

wxBEGIN_EVENT_TABLE(MainFrame, wxFrame)
	EVT_RIBBONBUTTONBAR_CLICKED(wxID_ADD, MainFrame::onRibbonButtonClicked)
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
    FSTreeItemData *currentNode =
        (FSTreeItemData *)fsTree->GetItemData(fsTree->GetSelection());
    if (currentNode->getFSNode()->GetType() == LabFS::Filesystem::NODE_FILE) {
        currentNode = (FSTreeItemData *)fsTree->GetItemData(
            fsTree->GetItemParent(fsTree->GetSelection()));
    }
    currentNode->getFSNode()->AddFileSubnode(fs.addFile(filename.ToStdString(), path));
    fsTree->AppendItem(fsTree->GetSelection(), filename);
}

void MainFrame::onRibbonButtonClicked(wxRibbonButtonBarEvent &event) {
    wxString message;
    switch (event.GetBar()->GetItemId(event.GetButton())) {
    case wxID_ADD:
        wxFileDialog openFileDialog(this, _("Выберите файл для добавления"));
        if (openFileDialog.ShowModal() == wxID_CANCEL)
            return;
		addFile(openFileDialog.GetFilename(), openFileDialog.GetPath().ToStdString());
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
        new wxRibbonPage(ribbonBar, wxID_ANY, wxT("Опции"), wxNullBitmap);

    fileManipPanel = new wxRibbonPanel(
        mainRibbonPage, wxID_ANY, wxT("Управление файлами"), wxNullBitmap,
        wxDefaultPosition, wxDefaultSize, wxRIBBON_PANEL_NO_AUTO_MINIMISE);

    addFileButtonBar = new wxRibbonButtonBar(fileManipPanel);
    addFileButtonBar->AddButton(wxID_ADD, wxT("Добавить файл"),
                                wxArtProvider::GetBitmap(wxART_FILE_OPEN,
                                                         wxART_TOOLBAR,
                                                         wxSize(32, 32)));
    addFileButtonBar->AddButton(
        wxID_DELETE, wxT("Удалить файл"),
        wxArtProvider::GetBitmap(wxART_DELETE, wxART_TOOLBAR, wxSize(32, 32)));

    ribbonBar->AddPageHighlight(ribbonBar->GetPageCount() - 1);
    ribbonBar->Realise();

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