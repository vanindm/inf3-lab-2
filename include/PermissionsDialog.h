#pragma once

#include <wx/wx.h>

#include "Filesystem.h"

class PermissionsDialog : public wxDialog
{
	wxCheckBox *rb, *rb1, *rb2, *rb3;
public:
	PermissionsDialog(const wxString& title);
	LabFS::Filesystem::Permissions GetValue() {
		int result = 
		(LabFS::Filesystem::Permissions::OWNER_READ) * (rb->GetValue()) |
		(LabFS::Filesystem::Permissions::OTHERS_READ) * rb1->GetValue() |
		(LabFS::Filesystem::Permissions::OWNER_WRITE) * rb2->GetValue() |
		(LabFS::Filesystem::Permissions::OTHERS_WRITE) * rb3->GetValue();
		return (LabFS::Filesystem::Permissions) result;
	}
};

PermissionsDialog::PermissionsDialog(const wxString & title)
		 : wxDialog(NULL, -1, title, wxDefaultPosition, wxSize(250, 230))
{

	wxPanel *panel = new wxPanel(this, -1);

	wxBoxSizer *vbox = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer *hbox = new wxBoxSizer(wxHORIZONTAL);

	wxStaticBox *st = new wxStaticBox(panel, -1, wxT("Разрешения"), 
		wxPoint(5, 5), wxSize(240, 150));
	rb = new wxCheckBox(panel, -1, 
		wxT("OWNER_READ"), wxPoint(15, 30), wxDefaultSize, wxRB_GROUP);

	rb1 = new wxCheckBox(panel, -1, 
		wxT("OTHERS_READ"), wxPoint(15, 55));
	rb2 = new wxCheckBox(panel, -1, 
		wxT("OWNER_WRITE"), wxPoint(15, 80));
	rb3 = new wxCheckBox(panel, -1, 
		wxT("OTHERS_WRITE"), wxPoint(15, 105));

	wxSizer* btnSizer = CreateStdDialogButtonSizer(wxOK | wxCANCEL);

	hbox->Add(btnSizer, 1);

	vbox->Add(panel, 1);
	vbox->Add(hbox, 0, wxALIGN_CENTER | wxTOP | wxBOTTOM, 10);

	SetSizer(vbox);

	Centre();
}