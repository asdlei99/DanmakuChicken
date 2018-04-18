﻿// OverlayDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "DanmakuChicken.h"
#include "OverlayDlg.h"


// COverlayDlg 对话框

IMPLEMENT_DYNAMIC(COverlayDlg, CDialog)

COverlayDlg::COverlayDlg(CWnd* pParent /*=NULL*/)
	: CDialog(COverlayDlg::IDD, pParent)
{
	// 全屏
	m_size.cx = GetSystemMetrics(SM_CXVIRTUALSCREEN);
	m_size.cy = GetSystemMetrics(SM_CYVIRTUALSCREEN);
	m_danmakuManager.m_danmakuBoxSize.Width = m_size.cx;
	m_danmakuManager.m_danmakuBoxSize.Height = m_size.cy;
}

COverlayDlg::~COverlayDlg()
{
}

void COverlayDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(COverlayDlg, CDialog)
	ON_WM_DESTROY()
END_MESSAGE_MAP()


// COverlayDlg 消息处理程序


BOOL COverlayDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// 全屏
	int x = GetSystemMetrics(SM_XVIRTUALSCREEN);
	int y = GetSystemMetrics(SM_YVIRTUALSCREEN);
	MoveWindow(x, y, m_size.cx, m_size.cy);

	// 创建DC
	m_bufferImg.Create(m_size.cx, m_size.cy, 32, CImage::createAlphaChannel);

	// 渲染线程
	m_renderThread = std::thread([this] {
		while (!m_stopThreads)
		{
			// 清除m_bufferImg
			for (int y = 0; y < m_size.cy; y++)
				memset(m_bufferImg.GetPixelAddress(0, y), 0, 4 * m_size.cx);
			// 渲染
			m_danmakuManager.RenderDanmakuSet(m_bufferImg.GetDC());
			m_bufferImg.ReleaseDC();
			UpdateUI();

			Sleep(1000 / 60); // 60fps
		}
	});

	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常:  OCX 属性页应返回 FALSE
}

void COverlayDlg::OnDestroy()
{
	CDialog::OnDestroy();

	m_stopThreads = TRUE;
	if (m_renderThread.joinable())
		m_renderThread.join();
}

// 显示m_bufferImg的图像
void COverlayDlg::UpdateUI()
{
	CRect wndRect;
	GetWindowRect(wndRect);
	CSize wndSize = wndRect.Size();

	POINT pointSrc = { 0, 0 };
	BLENDFUNCTION bf;
	bf.AlphaFormat = AC_SRC_ALPHA;
	bf.BlendFlags = 0;
	bf.BlendOp = AC_SRC_OVER;
	bf.SourceConstantAlpha = 255;

	CClientDC windowDC(this);
	CDC dc;
	dc.Attach(m_bufferImg.GetDC());
	UpdateLayeredWindow(&windowDC, &wndRect.TopLeft(), &wndSize, &dc, &pointSrc, 0, &bf, ULW_ALPHA);
	dc.Detach();
	m_bufferImg.ReleaseDC();
}
