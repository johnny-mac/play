#include "GsPacketListView.h"
#include "../../FrameDump.h"
#include "string_cast.h"
#include "string_format.h"
#include "win32/DpiUtils.h"
#include "../WinUtils.h"

CGsPacketListView::CGsPacketListView(HWND parentWnd, const RECT& rect)
    : m_frameDump(nullptr)
{
	Create(0, Framework::Win32::CDefaultWndClass::GetName(), _T(""), WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPCHILDREN,
	       Framework::Win32::CRect(0, 0, 1024, 768), parentWnd, nullptr);
	SetClassPtr();

	m_packetsTreeView = std::make_unique<Framework::Win32::CTreeView>(m_hWnd,
	                                                                  Framework::Win32::PointsToPixels(Framework::Win32::CRect(0, 0, 300, 300)),
	                                                                  TVS_LINESATROOT | TVS_HASBUTTONS | TVS_SHOWSELALWAYS | TVS_HASLINES);

	{
		LOGFONT fontInfo;
		HFONT packetsTreeViewFont = m_packetsTreeView->GetFont();
		GetObject(packetsTreeViewFont, sizeof(LOGFONT), &fontInfo);
		fontInfo.lfWeight = FW_BOLD;
		m_drawCallItemFont = CreateFontIndirect(&fontInfo);
	}

	m_prevDrawKickButton = std::make_unique<Framework::Win32::CButton>(_T("Prev Draw Kick"),
	                                                                   m_hWnd, Framework::Win32::PointsToPixels(Framework::Win32::CRect(0, 0, 100, 25)));
	m_nextDrawKickButton = std::make_unique<Framework::Win32::CButton>(_T("Next Draw Kick"),
	                                                                   m_hWnd, Framework::Win32::PointsToPixels(Framework::Win32::CRect(100, 0, 200, 25)));
}

CGsPacketListView::~CGsPacketListView()
{
}

void CGsPacketListView::SetFrameDump(CFrameDump* frameDump)
{
	m_frameDump = frameDump;

	m_packetsTreeView->SetSelection(NULL);
	m_packetsTreeView->SetRedraw(FALSE);
	m_packetsTreeView->DeleteAllItems();
	m_packetsTreeView->SetRedraw(TRUE);
	m_packetInfos.clear();
	m_writeInfos.clear();

	if(m_frameDump == nullptr) return;

	m_packetsTreeView->SetRedraw(false);

#ifdef DEBUGGER_INCLUDED

	m_packetInfos.reserve(m_frameDump->GetPackets().size());

	uint32 packetIndex = 0, cmdIndex = 0;
	const auto& drawingKicks = m_frameDump->GetDrawingKicks();
	for(const auto& packet : m_frameDump->GetPackets())
	{
		bool isRegisterPacket = packet.imageData.empty();

		auto lowerBoundIterator = drawingKicks.upper_bound(cmdIndex);
		auto upperBoundIterator = drawingKicks.lower_bound(cmdIndex + packet.registerWrites.size());

		int kickCount = static_cast<int>(std::distance(lowerBoundIterator, upperBoundIterator));

		std::tstring packetDescription;

		if(isRegisterPacket)
		{
			packetDescription = string_cast<std::tstring>(string_format("%d: Register Packet (Write Count: %d, Draw Count: %d, Path: %d)",
			                                                            packetIndex, packet.registerWrites.size(), kickCount, packet.metadata.pathIndex));
		}
		else
		{
			packetDescription = string_cast<std::tstring>(string_format("%d: Image Packet (Size: 0x%08X)",
			                                                            packetIndex, packet.imageData.size()));
		}

		TVINSERTSTRUCT insertStruct = {};
		insertStruct.hParent = TVI_ROOT;
		insertStruct.item.pszText = const_cast<LPWSTR>(packetDescription.c_str());
		insertStruct.item.cChildren = 1;
		insertStruct.item.lParam = packetIndex;
		insertStruct.item.mask = TVIF_TEXT | TVIF_PARAM;
		if(isRegisterPacket) insertStruct.item.mask |= TVIF_CHILDREN;
		HTREEITEM packetRootItem = m_packetsTreeView->InsertItem(&insertStruct);

		PACKETINFO packetInfo;
		packetInfo.cmdIndexStart = cmdIndex;
		packetInfo.treeViewItem = packetRootItem;
		m_packetInfos.push_back(packetInfo);

		cmdIndex += packet.registerWrites.size();
		packetIndex++;
	}

	m_writeInfos.resize(cmdIndex, WRITEINFO());

#endif //DEBUGGER_INCLUDED

	m_packetsTreeView->SetRedraw(true);

	RedrawWindow(*m_packetsTreeView, nullptr, nullptr, RDW_ERASE | RDW_FRAME | RDW_INVALIDATE | RDW_ALLCHILDREN);
}

uint32 CGsPacketListView::GetSelectedItemIndex() const
{
	HTREEITEM selectedItem = m_packetsTreeView->GetSelection();
	if(selectedItem == nullptr) return -1;
	TVITEM item = {};
	item.mask |= LVIF_PARAM;
	m_packetsTreeView->GetItem(selectedItem, &item);
	return GetItemIndexFromTreeViewItem(&item);
}

long CGsPacketListView::OnSize(unsigned int, unsigned int, unsigned int)
{
	auto clientRect = GetClientRect();
	Framework::Win32::CRect packetsTreeViewRect(0, Framework::Win32::PointsToPixels(30),
	                                            clientRect.Right(), clientRect.Bottom());
	m_packetsTreeView->SetSizePosition(packetsTreeViewRect);
	return TRUE;
}

long CGsPacketListView::OnCommand(unsigned short, unsigned short, HWND senderWnd)
{
	if(CWindow::IsCommandSource(m_prevDrawKickButton.get(), senderWnd))
	{
		OnPrevDrawKick();
	}
	else if(CWindow::IsCommandSource(m_nextDrawKickButton.get(), senderWnd))
	{
		OnNextDrawKick();
	}
	return TRUE;
}

LRESULT CGsPacketListView::OnNotify(WPARAM param, NMHDR* header)
{
	if(CWindow::IsNotifySource(m_packetsTreeView.get(), header))
	{
		switch(header->code)
		{
		case NM_CUSTOMDRAW:
			return OnPacketsTreeViewCustomDraw(reinterpret_cast<NMTVCUSTOMDRAW*>(header));
			break;
		case TVN_ITEMEXPANDING:
			OnPacketsTreeViewItemExpanding(reinterpret_cast<NMTREEVIEW*>(header));
			break;
		case TVN_SELCHANGED:
			OnPacketsTreeViewSelChanged(reinterpret_cast<NMTREEVIEW*>(header));
			break;
		case TVN_KEYDOWN:
			OnPacketsTreeViewKeyDown(reinterpret_cast<NMTVKEYDOWN*>(header));
			break;
		}
		return FALSE;
	}
	return FALSE;
}

long CGsPacketListView::OnCopy()
{
	HTREEITEM selectedItem = m_packetsTreeView->GetSelection();
	if(selectedItem == NULL) return TRUE;

	TVITEM treeViewItem = {};
	treeViewItem.mask = TVIF_PARAM | TVIF_HANDLE;
	m_packetsTreeView->GetItem(selectedItem, &treeViewItem);

	uint32 selectedItemIndex = GetItemIndexFromTreeViewItem(&treeViewItem);
	const auto& writeInfo = m_writeInfos[selectedItemIndex];
	const auto& registerWrite = writeInfo.registerWrite;

	auto text = string_format(_T("0x%02X -> 0x%016llX\r\n"),
	                          registerWrite.first, registerWrite.second);

	WinUtils::CopyStringToClipboard(text);

	return TRUE;
}

long CGsPacketListView::OnPacketsTreeViewCustomDraw(NMTVCUSTOMDRAW* customDraw)
{
	if(customDraw->nmcd.dwDrawStage == CDDS_PREPAINT)
	{
		return CDRF_NOTIFYITEMDRAW;
	}
	else if(customDraw->nmcd.dwDrawStage == CDDS_ITEMPREPAINT)
	{
		HTREEITEM drawItem = reinterpret_cast<HTREEITEM>(customDraw->nmcd.dwItemSpec);
		HTREEITEM drawItemParent = m_packetsTreeView->GetItemParent(drawItem);
		if(drawItemParent == nullptr) return CDRF_DODEFAULT;
		uint32 drawItemPacketIndex = m_packetsTreeView->GetItemParam<uint32>(drawItem);
		const auto& drawingKicks = m_frameDump->GetDrawingKicks();
		if(drawingKicks.find(drawItemPacketIndex) != std::end(drawingKicks))
		{
			SelectObject(customDraw->nmcd.hdc, m_drawCallItemFont);
			return CDRF_DODEFAULT | CDRF_NEWFONT;
		}
		else
		{
			return CDRF_DODEFAULT;
		}
	}
	else
	{
		return CDRF_DODEFAULT;
	}
}

uint32 CGsPacketListView::GetItemIndexFromTreeViewItem(TVITEM* item) const
{
	HTREEITEM itemParent = m_packetsTreeView->GetItemParent(item->hItem);
	if(itemParent == nullptr)
	{
		const auto& packetInfo = m_packetInfos[item->lParam];
		return packetInfo.cmdIndexStart;
	}
	else
	{
		return item->lParam;
	}
}

void CGsPacketListView::OnPacketsTreeViewItemExpanding(NMTREEVIEW* treeView)
{
	if((treeView->itemNew.state & TVIS_EXPANDEDONCE) == 0)
	{
		uint32 packetIndex = treeView->itemNew.lParam;
		const auto& packet = m_frameDump->GetPackets()[packetIndex];
		const auto& packetInfo = m_packetInfos[packetIndex];

		uint32 cmdIndex = packetInfo.cmdIndexStart;

		for(const auto& registerWrite : packet.registerWrites)
		{
			auto packetWriteDescription = CGSHandler::DisassembleWrite(registerWrite.first, registerWrite.second);
			auto treeItemText = string_format("%04X: %s", cmdIndex - packetInfo.cmdIndexStart, packetWriteDescription.c_str());
			HTREEITEM newItem = m_packetsTreeView->InsertItem(treeView->itemNew.hItem, string_cast<std::tstring>(treeItemText).c_str());

			auto& writeInfo = m_writeInfos[cmdIndex];
			writeInfo.treeViewItem = newItem;
			writeInfo.registerWrite = registerWrite;

			m_packetsTreeView->SetItemParam(newItem, cmdIndex++);
		}
	}
}

void CGsPacketListView::OnPacketsTreeViewSelChanged(NMTREEVIEW* treeView)
{
	uint32 selectedCmdIndex = GetItemIndexFromTreeViewItem(&treeView->itemNew);
	SELCHANGED_INFO selchangedInfo;
	memset(&selchangedInfo, 0, sizeof(SELCHANGED_INFO));
	selchangedInfo.code = NOTIFICATION_SELCHANGED;
	selchangedInfo.hwndFrom = m_hWnd;
	selchangedInfo.selectedCmdIndex = selectedCmdIndex;
	SendMessage(GetParent(), WM_NOTIFY, reinterpret_cast<WPARAM>(m_hWnd), reinterpret_cast<LPARAM>(&selchangedInfo));
}

void CGsPacketListView::OnPacketsTreeViewKeyDown(const NMTVKEYDOWN* keyDown)
{
	switch(keyDown->wVKey)
	{
	case 'C':
		if(GetAsyncKeyState(VK_CONTROL))
		{
			SendMessage(m_hWnd, WM_COPY, 0, 0);
		}
		break;
	}
}

void CGsPacketListView::GoToWrite(uint32 writeIndex)
{
	auto packetInfoIterator = std::lower_bound(std::begin(m_packetInfos), std::end(m_packetInfos), writeIndex,
	                                           [](const PACKETINFO& p1, uint32 i2) { return p1.cmdIndexStart < i2; });
	packetInfoIterator = std::prev(packetInfoIterator);
	assert(packetInfoIterator != std::end(m_packetInfos));

	const auto& packetInfo = *packetInfoIterator;

	m_packetsTreeView->Expand(packetInfo.treeViewItem);

	const auto& writeInfo = m_writeInfos[writeIndex];
	assert(writeInfo.treeViewItem != nullptr);
	m_packetsTreeView->SetSelection(writeInfo.treeViewItem);
}

void CGsPacketListView::OnPrevDrawKick()
{
	if(m_frameDump == nullptr) return;

	unsigned int selectedItemIndex = 0;

	HTREEITEM selectedItem = m_packetsTreeView->GetSelection();
	if(selectedItem != nullptr)
	{
		TVITEM treeViewItem;
		memset(&treeViewItem, 0, sizeof(TVITEM));
		treeViewItem.mask = TVIF_PARAM | TVIF_HANDLE;
		m_packetsTreeView->GetItem(selectedItem, &treeViewItem);
		selectedItemIndex = GetItemIndexFromTreeViewItem(&treeViewItem);
	}

	const auto& drawingKicks = m_frameDump->GetDrawingKicks();
	if(drawingKicks.empty()) return;

	auto prevKickIndexIterator = std::prev(drawingKicks.lower_bound(selectedItemIndex));
	if(prevKickIndexIterator == std::end(drawingKicks))
	{
		//Nothing to do here
		return;
	}

	GoToWrite(prevKickIndexIterator->first);
}

void CGsPacketListView::OnNextDrawKick()
{
	if(m_frameDump == nullptr) return;

	unsigned int selectedItemIndex = 0;

	HTREEITEM selectedItem = m_packetsTreeView->GetSelection();
	if(selectedItem != nullptr)
	{
		TVITEM treeViewItem;
		memset(&treeViewItem, 0, sizeof(TVITEM));
		treeViewItem.mask = TVIF_PARAM | TVIF_HANDLE;
		m_packetsTreeView->GetItem(selectedItem, &treeViewItem);
		selectedItemIndex = GetItemIndexFromTreeViewItem(&treeViewItem);
	}

	const auto& drawingKicks = m_frameDump->GetDrawingKicks();
	auto nextKickIndexIterator = drawingKicks.upper_bound(selectedItemIndex);
	if(nextKickIndexIterator == std::end(drawingKicks))
	{
		//Nothing to do here
		return;
	}

	GoToWrite(nextKickIndexIterator->first);
}
