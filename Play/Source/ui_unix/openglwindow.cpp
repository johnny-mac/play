#include "openglwindow.h"
#include <QResizeEvent>

OpenGLWindow::OpenGLWindow(QWindow* parent)
    : QWindow(parent)
{
	QSurfaceFormat format;
	format.setVersion(3, 2);
	format.setProfile(QSurfaceFormat::CoreProfile);
	format.setSwapBehavior(QSurfaceFormat::DoubleBuffer);

	setSurfaceType(QWindow::OpenGLSurface);
	setFormat(format);

	connect(this, SIGNAL(activeChanged()), this, SLOT(activeStateChanged()));
}

OpenGLWindow::~OpenGLWindow()
{
}

void OpenGLWindow::keyPressEvent(QKeyEvent* ev)
{
	emit keyDown(ev);
}

void OpenGLWindow::keyReleaseEvent(QKeyEvent* ev)
{
	emit keyUp(ev);
}

void OpenGLWindow::exposeEvent(QExposeEvent* ev)
{
	emit widthChanged(size().width());
	QWindow::exposeEvent(ev);
}

void OpenGLWindow::focusOutEvent(QFocusEvent* event)
{
	emit focusOut(event);
}
void OpenGLWindow::focusInEvent(QFocusEvent* event)
{
	emit focusIn(event);
}

void OpenGLWindow::activeStateChanged()
{
	if(isActive())
	{
		emit focusIn(new QFocusEvent(QEvent::FocusIn));
	}
	else
	{
		emit focusOut(new QFocusEvent(QEvent::FocusOut));
	}
}

void OpenGLWindow::mouseDoubleClickEvent(QMouseEvent* ev)
{
	emit doubleClick(ev);
}
