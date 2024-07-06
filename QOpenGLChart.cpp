#include "QOpenGLChart.h"

const double EARTHR_RADIUS_M = 6378137.0f;
QVector<QString> g_Order = { u8"红方",u8"蓝方",u8"绿方" };

QOpenGLChart::QOpenGLChart(QWidget *parent)
	: QOpenGLWidget(parent)
{
	setMouseTracking(true);
	Demo();
}

QOpenGLChart::~QOpenGLChart()
{
}

void QOpenGLChart::Demo()
{
	m_mapLines.insert(u8"红方", myline(u8"红方", Qt::red));
	m_mapLines.insert(u8"蓝方", myline(u8"蓝方", Qt::blue));
	m_mapLines.insert(u8"绿方", myline(u8"绿方", Qt::green));
	m_XOriginmin = 0;
	m_XOriginmax = 2.0f * M_PI;
	m_YOriginmin = 0;
	m_YOriginmax = 0;
	for (auto i = m_XOriginmin; i <= m_XOriginmax*1.01; i += (m_XOriginmax - m_XOriginmin)*0.001)
	{
		auto yValue = cos(i);
		m_YOriginmin = yValue < m_YOriginmin ? yValue : m_YOriginmin;
		m_YOriginmax = yValue > m_YOriginmax ? yValue : m_YOriginmax;
		m_mapLines[u8"红方"].data.push_back({ i, yValue });

		yValue = sin(i);
		m_YOriginmin = yValue < m_YOriginmin ? yValue : m_YOriginmin;
		m_YOriginmax = yValue > m_YOriginmax ? yValue : m_YOriginmax;
		m_mapLines[u8"蓝方"].data.push_back({ i, yValue });

		yValue = -cos(i);
		m_YOriginmin = yValue < m_YOriginmin ? yValue : m_YOriginmin;
		m_YOriginmax = yValue > m_YOriginmax ? yValue : m_YOriginmax;
		m_mapLines[u8"绿方"].data.push_back({ i, yValue });
	}
	m_Xmin = m_XOriginmin;
	m_Xmax = m_XOriginmax;
	m_YOriginmin = m_YOriginmin - abs(m_YOriginmin)*0.1;
	m_YOriginmax = m_YOriginmax + abs(m_YOriginmax)*0.1;
	m_Ymin = m_YOriginmin;
	m_Ymax = m_YOriginmax;
	m_bIsDrawXGrid = true;
	m_bIsDrawYGrid = true;
	auto hwscale = 22672.0f / 21600.0f;
	resize(1080, hwscale * 1080);
	m_fW2X = (m_Xmax - m_Xmin) / this->width();
	m_fH2Y = (m_Ymax - m_Ymin) / this->height();
}

QPair<double, double> QOpenGLChart::LonLatToXY(double lon, double lat)
{
	auto to_radians = [](double degrees)->double
	{
		return degrees*M_PI / 180.0;
	};
	auto lonToX = [&](double lon)->double
	{
		return lon*EARTHR_RADIUS_M*M_PI / 180.0;
	};
	auto latToY = [&](double lat)->double
	{
		if (lat > 85.0511287798)lat = 85.0511287798;
		if (lat < -85.0511287798) lat = -85.0511287798;
		return EARTHR_RADIUS_M*std::log(std::tan(0.5*(M_PI / 2 + to_radians(lat))))*(20037508.34 / EARTHR_RADIUS_M);
	};
	return QPair<double, double>(lonToX(lon), latToY(lat));
}

QPair<double, double> QOpenGLChart::XYToLonLat(double x, double y)
{
	auto XTolon = [&](double X)->double
	{
		return (x*180.0) / (M_PI*EARTHR_RADIUS_M);
	};
	auto YTolat = [&](double Y)->double
	{
		return (2 * atan(exp(y / (EARTHR_RADIUS_M*M_PI))) - M_PI / 2)*(180 / M_PI);
	};
	return QPair<double, double>(XTolon(x), YTolat(y));
}

void QOpenGLChart::DemoWGS84()
{
	auto EN = LonLatToXY(180.0, 90.0);
	auto WS = LonLatToXY(-180.0, -90.0);

	m_XOriginmin = WS.first;
	m_YOriginmin = WS.second;
	m_XOriginmax = EN.first;
	m_YOriginmax = EN.second;

	m_fMapScale = (m_XOriginmax - m_XOriginmin) / (m_YOriginmax - m_YOriginmin);

	m_Xmin = m_XOriginmin;
	m_Xmax = m_XOriginmax;
	m_Ymin = m_YOriginmin;
	m_Ymax = m_YOriginmax;

	m_XOpenGLmin = -1.0f;
	m_XOpenGLmax = 1.0f;
	m_YOpenGLmin = -1.0f;
	m_YOpenGLmax = 1.0f;

	m_iXInterCnt = 10;
	m_iYInterCnt = 10;
	m_bIsDrawXGrid = true;
	m_bIsDrawYGrid = true;
	auto hwscale = 22672.0f / 21600.0f;
	resize(1080, hwscale * 1080);
	m_fW2X = (m_Xmax - m_Xmin) / this->width();
	m_fH2Y = (m_Ymax - m_Ymin) / this->height();
}

void QOpenGLChart::UpdateEntity(myEntity entity)
{
    //m_mapEntitys.insert(entity.entity->id, entity);
}

void QOpenGLChart::RemoveEntity(int id)
{
	m_mapEntitys.remove(id);
}

void QOpenGLChart::DelEntity(int id)
{
	m_mapEntitys.remove(id);
}

void QOpenGLChart::SetAltData(int iAltWidth, int iAltHeight, double *geoTransform, float *elevation)
{
	m_geoTransform = geoTransform;
	m_elevation = elevation;
	m_iAltWidth = iAltWidth;
	m_iAltHeight = iAltHeight;
}

void QOpenGLChart::InsertLine(QString name, QColor color)
{
	if (m_mapLines.contains(name))return;
	m_mapLines.insert(name, myline(name, color));
}

void QOpenGLChart::InsertPoint(QString name, float x, float y)
{
	if (!m_mapLines.contains(name))return;
	m_XOriginmin = x < m_XOriginmin ? x : m_XOriginmin;
	m_XOriginmax = x > m_XOriginmax ? x : m_XOriginmax;
	m_Xmin = m_XOriginmin - abs(m_XOriginmin)*0.1;
	m_Xmax = m_XOriginmax + abs(m_XOriginmax)*0.1;
	m_YOriginmin = y < m_YOriginmin ? y : m_YOriginmin;
	m_YOriginmax = y > m_YOriginmax ? y : m_YOriginmax;
	m_Ymin = m_YOriginmin - abs(m_YOriginmin)*0.1;
	m_Ymax = m_YOriginmax + abs(m_YOriginmax)*0.1;
	m_mapLines[name].data.push_back({ x, y });
}

void QOpenGLChart::ClearAllPoints()
{
	for (auto iter = m_mapLines.begin(); iter != m_mapLines.end(); iter++)
	{
		iter.value().data.clear();
		iter.value().curselindex = -1;
	}
	m_Xmin = 0.0f;
	m_Xmax = 100.0f;
	m_XOriginmin = 0.0f;
	m_XOriginmax = 100.0f;
	m_Ymin = 0.0f;
	m_Ymax = 100.0f;
	m_YOriginmin = 0.0f;
	m_YOriginmax = 100.0f;
	update();
}

void QOpenGLChart::wheelEvent(QWheelEvent * event)
{
    auto delta = event->angleDelta().y();
	if (delta < 0)
	{
		auto dis = m_Xmax - m_Xmin;
		auto newdis = dis*1.1;
		auto changedis = newdis - dis;
		auto old_m_Xmax = m_Xmax;
		auto old_m_Xmin = m_Xmin;
		m_Xmax += abs(changedis) / 2;
		m_Xmin -= abs(changedis) / 2;
		dis = m_Ymax - m_Ymin;
		newdis = dis*1.1;
		changedis = newdis - dis;
		auto old_m_Ymax = m_Ymax;
		auto old_m_Ymin = m_Ymin;
		m_Ymax += abs(changedis) / 2;
		m_Ymin -= abs(changedis) / 2;

		//if (m_Ymax > m_YOriginmax)
		//	m_Ymax = m_YOriginmax;
		//if (m_Ymin < m_YOriginmin)
		//	m_Ymin = m_YOriginmin;
		//if (m_Xmax > m_XOriginmax)
		//	m_Xmax = m_XOriginmax;
		//if (m_Xmin < m_XOriginmin)
		//	m_Xmin = m_XOriginmin;
	}
	else
	{
		auto dis = m_Xmax - m_Xmin;
		auto newdis = dis*1.1;
		auto changedis = newdis - dis;
		auto old_m_Xmax = m_Xmax;
		auto old_m_Xmin = m_Xmin;
		m_Xmax -= abs(changedis) / 2;
		m_Xmin += abs(changedis) / 2;
		dis = m_Ymax - m_Ymin;
		newdis = dis*1.1;
		changedis = newdis - dis;
		auto old_m_Ymax = m_Ymax;
		auto old_m_Ymin = m_Ymin;
		m_Ymax -= abs(changedis) / 2;
		m_Ymin += abs(changedis) / 2;

		//if (m_Ymax > m_YOriginmax)
		//	m_Ymax = m_YOriginmax;
		//if (m_Ymin < m_YOriginmin)
		//	m_Ymin = m_YOriginmin;
		//if (m_Xmax > m_XOriginmax)
		//	m_Xmax = m_XOriginmax;
		//if (m_Xmin < m_XOriginmin)
		//	m_Xmin = m_XOriginmin;
	}
	m_fW2X = (m_Xmax - m_Xmin) / this->width();
	m_fH2Y = (m_Ymax - m_Ymin) / this->height();
	update();
}

void QOpenGLChart::initializeGL()
{
	initializeOpenGLFunctions();
	QOpenGLFunctions*glFunctions = QOpenGLContext::currentContext()->functions();
	//glFunctions->glClearColor(163.0f / 255.0f, 205.0f / 255.0f, 255.0f / 255.0f, 0.5f);
}

void QOpenGLChart::resizeGL(int w, int h)
{
	glViewport(0, 0, w, h);
	static auto lastpos = this->pos();
	auto curpos = this->pos();
	if (curpos.x() == lastpos.x())
		m_Xmax = m_Xmin + m_fW2X*w;
	else
		m_Xmin = m_Xmax - m_fW2X*w;
	if (curpos.y() == lastpos.y())
		m_Ymin = m_Ymax - m_fH2Y*h;
	else
		m_Ymax = m_Ymin + m_fH2Y*h;
	lastpos = this->pos();
	//if (m_Xmax > m_XOriginmax)
	//	m_Xmax = m_XOriginmax;
	//if (m_Ymax > m_YOriginmax)
	//	m_Ymax = m_YOriginmax;
}

void QOpenGLChart::mousePressEvent(QMouseEvent * event)
{
	if (event->button() == Qt::LeftButton)
	{
		m_lastMousePos = event->pos();
		m_bDragging = true;
		m_setPressEntitys.unite(m_setHoverEntitys);
	}
	else if (event->button() == Qt::RightButton)
	{
		m_setPressEntitys.subtract(m_setHoverEntitys);
	}
}

void QOpenGLChart::mouseMoveEvent(QMouseEvent * event)
{
	if (m_bDragging)
	{
		QPoint currentMousePos = event->pos();
		QPoint offset = currentMousePos - m_lastMousePos;

		float x = offset.x();
		float y = offset.y();
		auto opengldisX = (2.0f*x / float(this->width()));
		auto maxopenglXdis = m_XOpenGLmax - m_XOpenGLmin;
		auto maxXValueDis = m_Xmax - m_Xmin;
		auto XChangeDis = maxXValueDis*opengldisX / maxopenglXdis;

		auto opengldisY = (2.0f*y / float(this->height()));
		auto maxopenglYdis = m_YOpenGLmax - m_YOpenGLmin;
		auto maxYValueDis = m_Ymax - m_Ymin;
		auto YChangeDis = maxYValueDis*opengldisY / maxopenglYdis;

		//if (m_Xmax - XChangeDis <= m_XOriginmax&&
		//	m_Xmax - XChangeDis >= m_XOriginmin&&
		//	m_Xmin - XChangeDis <= m_XOriginmax&&
		//	m_Xmin - XChangeDis >= m_XOriginmin)
		{
			m_Xmax -= XChangeDis;
			m_Xmin -= XChangeDis;
		}

		//if (m_Ymax + YChangeDis <= m_YOriginmax&&
		//	m_Ymax + YChangeDis >= m_YOriginmin&&
		//	m_Ymin + YChangeDis <= m_YOriginmax&&
		//	m_Ymin + YChangeDis >= m_YOriginmin)
		{
			m_Ymax += YChangeDis;
			m_Ymin += YChangeDis;
		}

		m_lastMousePos = currentMousePos;
	}
	else
	{
		for (auto line = m_mapLines.begin(); line != m_mapLines.end(); line++)
		{
			line.value().curselindex = -1;
		}
		QPoint currentMousePos = event->pos();
		auto openglpos = WidgetPosToOpenGLPos(currentMousePos.x(), currentMousePos.y());
		if (openglpos.first > m_XOpenGLmin&&
			openglpos.first  < m_XOpenGLmax&&
			openglpos.second > m_YOpenGLmin&&
			openglpos.second < m_YOpenGLmax)
		{
			auto opengldx = openglpos.first;
			auto xp = (opengldx - m_XOpenGLmin) / (m_XOpenGLmax - m_XOpenGLmin);
			auto valuex = m_Xmin + (m_Xmax - m_Xmin)*xp;
			for (auto line = m_mapLines.begin(); line != m_mapLines.end(); line++)
			{
				int index = 0;
				for (auto point = line->data.begin(); point != line->data.end(); point++, index++)
				{
					if (point->first > valuex)
					{
						if (point == line->data.begin())
						{
							auto dis = point->first - valuex;
							if (dis < (m_Xmax - m_Xmin)*0.01)
							{
								line.value().curselindex = index;
							}
							break;
						}
						else
						{
							auto prevpoint = point - 1;
							if (prevpoint->first < valuex)
							{
								auto dis0 = point->first - valuex;
								auto dis1 = valuex - prevpoint->first;
								if (dis0 < (m_Xmax - m_Xmin)*0.01)
								{
									line.value().curselindex = index;
								}
								else if (dis1 < (m_Xmax - m_Xmin)*0.01)
								{
									line.value().curselindex = index - 1;
								}
								break;
							}
						}
					}
				}
			}
		}
	}
	m_setHoverEntitys.clear();
	for (auto iter : m_mapEntitys)
	{
		if (iter.openglx != -1)
		{
			QPoint currentMousePos = event->pos();
			auto openglpos = WidgetPosToOpenGLPos(currentMousePos.x(), currentMousePos.y());
			if (abs(iter.openglx - openglpos.first) < 0.01f&&
				abs(iter.opengly - openglpos.second) < 0.01f)
			{
                //m_setHoverEntitys.insert(iter.entity->id);
			}
		}
	}
	update();
}

void QOpenGLChart::mouseDoubleClickEvent(QMouseEvent * event)
{
	static float center_X = (14199793.0 - 12700419.0) / 2 + 12700419.0;
	static float center_Y = (9943060.00 - 7039607.50) / 2 + 7039607.50;
	m_fW2X = 1249.61658;
	auto disx = m_fW2X*this->width();
	m_fH2Y = 3925.79028;
	auto disy = m_fH2Y*this->height();
	m_Xmin = center_X - disx / 2;
	m_Xmax = center_X + disx / 2;
	m_Ymin = center_Y - disy / 2;
	m_Ymax = center_Y + disy / 2;
	update();
	if (this->isFullScreen())
		showNormal();
	else
		showFullScreen();
}

void QOpenGLChart::mouseReleaseEvent(QMouseEvent * event)
{
	if (event->button() == Qt::LeftButton)
	{
		m_bDragging = false;
	}
}

float QOpenGLChart::WidgetDisToOpenGLDis(float value, int type)
{
	float width = this->width();
	float height = this->height();
	if (type == 0)
	{
		return abs(value)* 2.0f / width;
	}
	else
	{
		return abs(value)* 2.0f / height;
	}
}

float QOpenGLChart::OpenGLDisToWidgetDis(float value, int type)
{
	float width = this->width();
	float height = this->height();
	if (type == 0)
	{
		return abs(value)* width / 2.0f;
	}
	else
	{
		return abs(value)* height / 2.0f;
	}
}

QPair<float, float> QOpenGLChart::WidgetPosToOpenGLPos(int x, int y)
{
	float width = this->width();
	float height = this->height();
	float fx = x;
	float fy = y;
	float w = ((x / width) * 2.0f) - 1.0f;
	float h = 1.0f - ((y / height) * 2.0f);
	return{ w,h };
}

QPair<int, int> QOpenGLChart::OpenGLPosToWidgetPos(float x, float y)
{
	int width = this->width();
	int height = this->height();
	int w = ((x - (-1.0f)) / 2.0f)*width;
	int h = height - ((y - (-1.0f)) / 2.0f)*height;
	return{ w,h };
}

void QOpenGLChart::DrawTextByQPainter(float x, float y, QString txt, QColor color, QFont font)
{
	QPainter painter(this);
	painter.beginNativePainting();
	auto pos = OpenGLPosToWidgetPos(x + 0, y - 0);
	int w = pos.first;
	int h = pos.second;
	painter.begin(this);
	painter.setPen(color);
	painter.setFont(font);
	QFontMetrics fontMetrics(font);
    auto textWidth = fontMetrics.width(txt);
	painter.drawText(w, h, txt);
    painter.end();
	painter.endNativePainting();
}

void QOpenGLChart::DrawMultiLineTextByQPainter(float x, float y, QStringList txts, QList<QColor> colors, QFont font, float alphaf)
{
	if (txts.isEmpty())return;

	int allHeight = 0;
	int maxWidth = 0;
	QFontMetrics fontMetrics(font);
	int singleHeight = fontMetrics.height();
	for (auto txt : txts)
	{
        auto textWidth = fontMetrics.width(txt);
		maxWidth = maxWidth > textWidth ? maxWidth : textWidth;
		allHeight += singleHeight;
	}

	auto color = colors.begin();

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glBegin(GL_POLYGON);
	glColor4f(125.0f / 255.0f, 125.0f / 255.0f, 125.0 / 255.0f, alphaf / 255.0f);
	float xleft, xright, ytop, ybottom;
	float xoffset = WidgetDisToOpenGLDis(10, 0);
	float yoffset = WidgetDisToOpenGLDis(10, 1);
	xleft = x - WidgetDisToOpenGLDis(maxWidth / 2, 0) - xoffset;
	xright = xleft + WidgetDisToOpenGLDis(maxWidth, 0) + 2 * xoffset;
	ytop = y + WidgetDisToOpenGLDis(singleHeight, 1) + yoffset;
	ybottom = ytop - WidgetDisToOpenGLDis(allHeight, 1) - 2 * yoffset;
	glVertex2f(xleft, ytop);
	glVertex2f(xright, ytop);
	glVertex2f(xright, ybottom);
	glVertex2f(xleft, ybottom);
	glEnd();

	auto pos = OpenGLPosToWidgetPos(x, y);
	auto curx = pos.first;
	auto cury = pos.second;

	for (auto txt : txts)
	{
		QPainter painter(this);
		painter.beginNativePainting();
		painter.begin(this);
		painter.setPen(*color);
		painter.setFont(font);
        auto textWidth = fontMetrics.width(txt);
		maxWidth = maxWidth > textWidth ? maxWidth : textWidth;
		painter.drawText(curx - textWidth / 2, cury, txt);
		painter.end();
		painter.endNativePainting();
		color++;
		if (color == colors.end())
			color--;
		cury += singleHeight;
		allHeight += singleHeight;
	}
}

void QOpenGLChart::DrawTextByQPainter(float x, float y, QStringList txts, QList<QColor> colors, QFont font)
{
	if (txts.isEmpty())return;
	if (txts.size() != colors.size())return;
	auto pos = OpenGLPosToWidgetPos(x + 0, y - 0);
	auto curx = pos.first;
	auto cury = pos.second;
	auto color = colors.begin();
	for (auto txt : txts)
	{
		QPainter painter(this);
		painter.beginNativePainting();
		painter.begin(this);
		painter.setPen(*color);
		painter.setFont(font);
		QFontMetrics fontMetrics(font);
        auto textWidth = fontMetrics.width(txt);
		painter.drawText(curx - textWidth / 2, cury, txt);
		painter.end();
		painter.endNativePainting();
		color++;
		curx += textWidth;
	}
}

void QOpenGLChart::DrawCube(float x, float y, QColor color, float r)
{
	float w = this->width();
	float h = this->height();
	auto rx = 2.0f*r / w;
	auto ry = 2.0f*r / h;
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glBegin(GL_POLYGON);
	float red = color.red();
	float green = color.green();
	float blue = color.blue();
	glColor4f(red / 255.0f, green / 255.0f, blue / 255.0f, 200.0f / 255.0f);
	float quadsx = x - rx / 2;
	float quadsy = y + ry / 2;
	glVertex2f(quadsx, quadsy);
	glVertex2f(quadsx + rx, quadsy);
	glVertex2f(quadsx + rx, quadsy - ry);
	glVertex2f(quadsx, quadsy - ry);
	glEnd();
}

void QOpenGLChart::DrawLinesTitle(float x, float y, QStringList txts, QList<QColor> colors, QFont font)
{
	if (txts.isEmpty())return;
	if (txts.size() != colors.size())return;
	float xstart = x;
	float ystart = y;

	auto color = colors.begin();
	static float r = 28;
	float w = this->width();
	float h = this->height();
	auto rx = 2.0f*r / w;
	auto ry = 2.0f*r / h;
	for (auto txt : txts)
	{
		glBegin(GL_POLYGON);
		float red = color->red();
		float green = color->green();
		float blue = color->blue();
		glColor4f(red / 255.0f, green / 255.0f, blue / 255.0f, 0.5f);
		float quadsx = xstart - rx / 2;
		float quadsy = ystart + ry / 2;
		glVertex2f(quadsx, quadsy);
		glVertex2f(quadsx + rx, quadsy);
		glVertex2f(quadsx + rx, quadsy - ry);
		glVertex2f(quadsx, quadsy - ry);
		glEnd();

		glBegin(GL_LINE_LOOP);
		glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
		glVertex2f(quadsx, quadsy);
		glVertex2f(quadsx + rx, quadsy);
		glVertex2f(quadsx + rx, quadsy - ry);
		glVertex2f(quadsx, quadsy - ry);
		glEnd();

		xstart += rx;
		QPainter painter(this);
		painter.beginNativePainting();
		painter.begin(this);
		painter.setPen({ 255,255,255,255 });
		painter.setFont(font);
		QFontMetrics fontMetrics(font);
        auto textWidth = fontMetrics.width(txt);
		auto textHeight = fontMetrics.height();
		auto otextHeight = 2.0f*float(textHeight) / h;
		auto pos = OpenGLPosToWidgetPos(xstart, ystart - otextHeight / 3);
		auto curx = pos.first;
		auto cury = pos.second;
		painter.drawText(curx, cury, txt);
		painter.end();
		painter.endNativePainting();
		color++;
		xstart += rx + 2.0f*float(textWidth) / w;
	}
}

void QOpenGLChart::DrawCube()
{
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(0.0, 0.0, 5.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
	glBegin(GL_TRIANGLES);
	GLUquadricObj* quadric = gluNewQuadric();
	gluQuadricNormals(quadric, GLU_SMOOTH);
	gluQuadricTexture(quadric, GLU_FALSE);
	gluSphere(quadric, 1.0, 30, 30);
	gluDeleteQuadric(quadric);
	glColor4f(2.0f, 1.0f, 0.0f, 1.0f);
	glVertex2f(-0.5f, -0.4f);
	glVertex2f(0.5f, -0.4f);
	glVertex2f(0.5f, 0.3f);
	glVertex2f(-0.5f, 0.6f);
	glVertex2f(-0.4f, 1.2f);
	glEnd();
	return;
	GLfloat vectices[] =
	{
		-0.5f,-0.5f,-0.5f,
		0.5f,-0.5f,-0.5f,
		0.5f,0.5f,-0.5f,
		-0.5f,0.5f,-0.5f,
		-0.5f,-0.5f,0.5f,
		0.5f,-0.5f,0.5f,
		0.5f,0.5f,0.5f,
		-0.5f,0.5f,0.5f
	};
	GLuint indices[] =
	{
		0,1,2,3,
		1,5,6,2,
		5,4,7,6,
		4,0,3,7,
		4,5,1,0,
		3,2,6,7
	};
	//QMatrix4x4 modelViewMatrix;
	//modelViewMatrix.translate(0.0f, 0.0f, -3.0f);
	//float rotationAngle = 0.5f;
	//QVector3D rotationAxis = QVector3D(0.0f, 1.0f, 0.0f);
	//modelViewMatrix.rotate(rotationAngle, rotationAxis);
	//glMatrixMode(GL_MODELVIEW);
	//glLoadMatrixf(modelViewMatrix.constData());
	glRotatef(0.1f, 1, 0, 0);
	glRotatef(0.0f, 0, 1, 0);
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, 0, vectices);
	glDrawElements(GL_QUADS, 24, GL_UNSIGNED_INT, indices);
	glDisableClientState(GL_VERTEX_ARRAY);
	glLoadIdentity();
}

void QOpenGLChart::DrawMap()
{
	if (!m_elevation)return;
	float r = 5;
	float w = this->width();
	float h = this->height();
	auto rx = 2.0f*r / w;
	auto ry = 2.0f*r / h;
	static bool isInitMax = false;
	static bool isInitMin = false;
	static float maxAlt = -1;
	static float minAlt = -1;
	if (!isInitMax)
	{
		for (unsigned long long i = 0;
			i < (unsigned long long)m_iAltWidth*(unsigned long long)m_iAltHeight; i++)
		{
			auto value = m_elevation[i];
			if (!isInitMax)
			{
				maxAlt = value;
				isInitMax = true;
			}
			else
			{
				maxAlt = maxAlt > value ? maxAlt : value;
			}
			if (!isInitMin)
			{
				minAlt = value;
				isInitMin = true;
			}
			else
			{
				minAlt = minAlt < value ? minAlt : value;
			}
		}
	}
/*
	for (float x = m_XOpenGLmin; x < m_XOpenGLmax + rx; x += rx)
	{
		for (float y = m_YOpenGLmin; y < m_YOpenGLmax + ry; y += ry)
		{
			auto xp = (x - m_XOpenGLmin) / (m_XOpenGLmax - m_XOpenGLmin);
			auto xValue = m_Xmin + xp*(m_Xmax - m_Xmin);
			auto yp = (y - m_YOpenGLmin) / (m_YOpenGLmax - m_YOpenGLmin);
			auto yValue = m_Ymin + yp*(m_Ymax - m_Ymin);
			auto lonloat = XYToLonLat(xValue, yValue);

			eIContentView* pGlobeViewer =
				eService::GetInstance()->m_pContentManager->ActivedContentView();
			eGlobeViewer* globeViewer = static_cast<eGlobeViewer*>(pGlobeViewer);
			Vector2d pos = { lonloat.first ,lonloat.second };
			pos = globeViewer->GetGlobe()->GetSpatialReference()->Forward(pos);

			auto pixelX = (int)((pos.x - m_geoTransform[0]) / m_geoTransform[1]);
			auto pixelY = (int)((pos.y - m_geoTransform[3]) / m_geoTransform[5]);
			if (pixelY*m_iAltWidth + pixelX < m_iAltWidth*m_iAltHeight&&
				pixelY*m_iAltWidth + pixelX >= 0)
			{
				auto alt = m_elevation[pixelY*m_iAltWidth + pixelX];

				if (alt > 0)
				{
					auto altpr = (alt - minAlt) / (maxAlt - minAlt);
					DrawCube(x, y, QColor(altpr * 255, altpr * 255, altpr * 255, 0.8f), r);
				}
				else
				{
					auto altpb = (alt - 0) / (minAlt - 0);
					DrawCube(x, y, QColor(163, 205, 255 - altpb * 255, 0.8f), r);
				}
			}
		}
    }*/
}

void QOpenGLChart::DrawFightTime()
{
	auto font = QFont("微软雅黑", 13, QFont::Bold);
	QStringList strs;
	QList<QColor> colors;
	colors.push_back({ 0,0,0,255 });
	colors.push_back({ 0,0,0,255 });
	QDateTime dateTime = QDateTime::currentDateTime();
	QString realTime = dateTime.toString(QObject::tr("天文时间 : yyyy.MM.dd hh:mm:ss"));
	strs << m_strfightTime;
	strs << realTime;
	DrawMultiLineTextByQPainter(0.0f, 0.95f, strs, colors, font, 125.0f);
}

void QOpenGLChart::paintGL()
{
	glEnable(GL_MULTISAMPLE);
	QOpenGLFunctions*glFunctions = QOpenGLContext::currentContext()->functions();
	glFunctions->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	float scaleFactor = m_scale;
	glScalef(scaleFactor, scaleFactor, 1.0f);
	float tickSize = m_YOpenGLmax - m_YOpenGLmin;
	float tickInterval = (m_XOpenGLmax - m_XOpenGLmin) / m_iXInterCnt;
	auto font = QFont("微软雅黑", 12, QFont::Bold);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glClearColor(163.0f / 255.0f, 205.0f / 255.0f, 255.0f / 255.0f, 0.0f);
	//DrawMap();

	if (m_bIsDrawXGrid)
	{
		for (float x = m_XOpenGLmin + tickInterval; x < m_XOpenGLmax; x += tickInterval)
		{
			glLineWidth(2.0f);
			glBegin(GL_LINES);
			glColor3f(0.0f / 255.0f, 0 / 255.0f, 0 / 255.0f);
			glVertex2f(x, m_YOpenGLmin);
			glVertex2f(x, m_YOpenGLmin + tickSize);
			glEnd();
			auto xp = (x - m_XOpenGLmin) / (m_XOpenGLmax - m_XOpenGLmin);
			auto xValue = m_Xmin + xp*(m_Xmax - m_Xmin);
			auto str = QString::number(XYToLonLat(xValue, 0).first, 'f', 2);
			if (m_bIsDateTimeMode)
			{
				str = QDateTime::fromMSecsSinceEpoch(xValue).toString("HH:mm:ss");
			}
			str = QString::number(xValue,'f',2);
            DrawTextByQPainter(x*m_scale, (m_YOpenGLmin + 0.05f)*m_scale, str, QColor(0, 0, 0), font);
		}
	}

	if (m_bIsDrawYGrid)
	{
		tickSize = m_XOpenGLmax - m_XOpenGLmin;
		tickInterval = (m_YOpenGLmax - m_YOpenGLmin) / m_iYInterCnt;
		for (float y = m_YOpenGLmin + tickInterval; y <= m_YOpenGLmax; y += tickInterval)
		{
			glLineWidth(2.0f);
			glBegin(GL_LINES);
			glColor3f(0.0f / 255.0f, 0.0f / 255.0f, 0.0f / 255.0f);
			glVertex2f(m_XOpenGLmin, y);
			glVertex2f(m_XOpenGLmin + tickSize, y);
			glEnd();
			auto yp = (y - m_YOpenGLmin) / (m_YOpenGLmax - m_YOpenGLmin);
			auto yValue = m_Ymin + yp*(m_Ymax - m_Ymin);
			auto str = QString::number(XYToLonLat(0, yValue).second, 'f', 2);
			str = QString::number(yValue, 'f', 2);
            DrawTextByQPainter((m_XOpenGLmin)*m_scale, y*m_scale, str, QColor(0, 0, 0), font);
		}
	}

	for (auto line = m_mapLines.begin(); line != m_mapLines.end(); line++)
	{
		glLineWidth(4.0f);
		glBegin(GL_LINE_STRIP);
		float red = line.value().color.red();
		float green = line.value().color.green();
		float blue = line.value().color.blue();
		glColor4f(red / 255.0f, green / 255.0f, blue / 255.0f, 0.75f);
		for (auto iter : line.value().data)
		{
			auto x = iter.first;
			auto y = iter.second;
			if (x<m_Xmin || x>m_Xmax)
				continue;
			auto xp = (x - m_Xmin) / (m_Xmax - m_Xmin);
			auto xOpengGL = m_XOpenGLmin + (m_XOpenGLmax - m_XOpenGLmin)*xp;
			auto yp = (y - m_Ymin) / (m_Ymax - m_Ymin);
			auto yOpengGL = m_YOpenGLmin + (m_YOpenGLmax - m_YOpenGLmin)*yp;
			if (xOpengGL < m_XOpenGLmin)
				continue;
			if (xOpengGL > m_XOpenGLmax)
				continue;
			if (yOpengGL > m_YOpenGLmax)
				continue;
			if (yOpengGL < m_YOpenGLmin)
				continue;
			glVertex2f(xOpengGL, yOpengGL);
		}
		glEnd();
	}
	QStringList str;
	QList<QColor> colors;
	for (auto iter : g_Order)
	{
		if (m_mapLines.contains(iter))
		{
			auto line = m_mapLines[iter];
			str << line.name + " ";
			colors << line.color;
			if (line.curselindex != -1)
			{
				auto iter = (line.data)[line.curselindex];
				auto x = iter.first;
				auto y = iter.second;
				auto xp = (x - m_Xmin) / (m_Xmax - m_Xmin);
				auto xOpengGL = m_XOpenGLmin + (m_XOpenGLmax - m_XOpenGLmin)*xp;
				auto yp = (y - m_Ymin) / (m_Ymax - m_Ymin);
				auto yOpengGL = m_YOpenGLmin + (m_YOpenGLmax - m_YOpenGLmin)*yp;
				auto strx = QString::number(x, 'f', 2);
				auto stry = QString::number(y, 'f', 2);
				auto strv = QString("(%1,%2)").arg(strx).arg(stry);
				DrawTextByQPainter(xOpengGL, yOpengGL, strv, line.color, font);
				DrawTextByQPainter(m_XOpenGLmin, yOpengGL, stry, { 125,125,0 }, font);
				DrawTextByQPainter(xOpengGL, m_YOpenGLmin, strx, { 125,125,0 }, font);
				glBegin(GL_LINE_STRIP);
				float red = 125;
				float green = 125;
				float blue = 0;
				glColor4f(red / 255.0f, green / 255.0f, blue / 255.0f, 0.75f);
				glVertex2f(m_XOpenGLmin, yOpengGL);
				glVertex2f(xOpengGL, yOpengGL);
				glVertex2f(xOpengGL, m_YOpenGLmin);
				glEnd();
			}
		}
	}
	DrawLinesTitle((m_XOpenGLmin + ((m_XOpenGLmax - m_XOpenGLmin) / 2))*m_scale, m_YOpenGLmax*1.05*m_scale, str, colors, font);
	font = QFont("微软雅黑", 13, QFont::Bold);
/*
	for (auto entity = m_mapEntitys.begin(); entity != m_mapEntitys.end(); entity++)
	{
		auto wgs84pos = LonLatToXY(entity->lon, entity->lat);
		entity->wgs84x = wgs84pos.first;
		entity->wgs84y = wgs84pos.second;
		auto x = entity->wgs84x;
		auto y = entity->wgs84y;
		auto xp = (x - m_Xmin) / (m_Xmax - m_Xmin);
		auto xOpengGL = m_XOpenGLmin + (m_XOpenGLmax - m_XOpenGLmin)*xp;
		auto yp = (y - m_Ymin) / (m_Ymax - m_Ymin);
		auto yOpengGL = m_YOpenGLmin + (m_YOpenGLmax - m_YOpenGLmin)*yp;
		entity->openglx = xOpengGL;
		entity->opengly = yOpengGL;
		if (xOpengGL < m_XOpenGLmin)
			continue;
		if (xOpengGL > m_XOpenGLmax)
			continue;
		if (yOpengGL > m_YOpenGLmax)
			continue;
		if (yOpengGL < m_YOpenGLmin)
			continue;
		DrawCube(xOpengGL, yOpengGL, entity->color);
		if (m_setHoverEntitys.contains(entity->entity->id) ||
			m_setPressEntitys.contains(entity->entity->id))
		{
			QString str_cgftype;
			if (theEquipDB.GetCgfTypes()->contains(entity->entity->cgftype))
				str_cgftype = (theEquipDB.GetCgfTypes()->value(entity->entity->cgftype))->name;
			QList<QColor> colors;
			colors.push_back(entity->color);
			QStringList strs;
			float alt;
			if (m_geoTransform)
			{
				auto pixelX = (int)((entity->wgs84x - m_geoTransform[0]) / m_geoTransform[1]);
				auto pixelY = (int)((entity->wgs84y - m_geoTransform[3]) / m_geoTransform[5]);
				if (pixelY*m_iAltWidth + pixelX < m_iAltWidth*m_iAltHeight&&
					pixelY*m_iAltWidth + pixelX >= 0)
				{
					alt = m_elevation[int(pixelY*m_iAltWidth + pixelX)];
				}
			}
			strs << entity->name
				<< QString("[生命值:%1]").arg(entity->entity->damagePercent)
				<< QString("[%1,%2]").arg(entity->lon).arg(entity->lat)
				<< str_cgftype
				<< QString("[指挥官:%1]")
				.arg(QString::fromLocal8Bit(entity->entity->commanderName.c_str()))
				<< QString("[油料%1磅]").arg(entity->entity->fFuel);
                << QString("[高程%1米]").arg(alt);
			DrawMultiLineTextByQPainter(entity->openglx, entity->opengly, strs, colors, font);
		}
    }
*/
    //DrawFightTime();
	glDisable(GL_MULTISAMPLE);
}

