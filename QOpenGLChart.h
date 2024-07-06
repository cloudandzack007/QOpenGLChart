#pragma once

#include <QOpenGLFunctions>
#include <QOpenGLWidget>
#include <qmath.h>
#include <QDialog>
#include <QSet>
#include <QPainter>
#include <QWheelEvent>
#include <QMatrix4x4>
#include <QTimer>
#include <GL/glu.h>
#include <QDateTime>
#include <cmath>
#include <QResizeEvent>
using namespace std;

using mypoint = QPair<float, float>;
using linedata = QVector<mypoint>;
struct myline
{
	linedata data;
	int curselindex;
	QColor color;
	QString name;
	myline()
	{};
	myline(QString name, QColor color)
	{
		data.clear();
		curselindex = -1;
		this->name = name;
		this->color = color;
	};
};

struct myEntity
{
	double lon;
	double lat;
	double wgs84x;
	double wgs84y;
	float openglx = -1;
	float opengly = -1;
	QColor color;
	QString name;
    /*PTSimEntity*entity;
	myEntity(PTSimEntity*entity)
	{
		this->entity = entity;
		this->lon = entity->position.x;
		this->lat = entity->position.y;
		this->name = QString::fromLocal8Bit(entity->name);
		this->color = g_SimSituationManager->GetColor(SimData::SideEnum(entity->side));
    };*/
};

class QOpenGLChart : public QOpenGLWidget, protected QOpenGLFunctions
{
	Q_OBJECT

public:
	QOpenGLChart(QWidget *parent = Q_NULLPTR);
	~QOpenGLChart();
	void Demo();
	QPair<double, double> LonLatToXY(double lon, double lat);
	QPair<double, double> XYToLonLat(double x, double y);
	void DemoWGS84();
	void InsertLine(QString name, QColor color);
	void InsertPoint(QString name, float x, float y);
	void ClearAllPoints();
	void SetDateTimeMode(bool mode) { m_bIsDateTimeMode = mode; };
	void UpdateEntity(myEntity entity);
	void RemoveEntity(int id);
	void DelEntity(int id);
	void SetAltData(int, int, double*, float*);
protected:
	virtual void wheelEvent(QWheelEvent *event);
	virtual void initializeGL();
	virtual void resizeGL(int w, int h);
	virtual void mousePressEvent(QMouseEvent *event);
	virtual void mouseReleaseEvent(QMouseEvent *event);
	float WidgetDisToOpenGLDis(float value, int type);
	float OpenGLDisToWidgetDis(float value, int type);
	QPair<float, float> WidgetPosToOpenGLPos(int x, int y);
	virtual void mouseMoveEvent(QMouseEvent *event);
	virtual void mouseDoubleClickEvent(QMouseEvent *event);
	QPair<int, int> OpenGLPosToWidgetPos(float x, float y);
	void DrawTextByQPainter(float x, float y, QString txt, QColor color, QFont font);
	void DrawMultiLineTextByQPainter(float x, float y, QStringList txts, QList<QColor> colors, QFont font, float alphaf = 125.0f);
	void DrawTextByQPainter(float x, float y, QStringList txts, QList<QColor> colors, QFont font);
	void DrawCube(float x, float y, QColor color, float r = 15);
	void DrawLinesTitle(float x, float y, QStringList txts, QList<QColor> colors, QFont font);
	void DrawCube();
	void DrawMap();
	void DrawFightTime();
	virtual void paintGL();

private:
	int m_iAltWidth = 0;
	int m_iAltHeight = 0;
	double* m_geoTransform = Q_NULLPTR;
	float* m_elevation = Q_NULLPTR;
	bool m_bIsDateTimeMode = false;
	QPoint m_lastMousePos;
	bool m_bDragging = false;
	float m_scale = 1.0f;
	float m_Xmin = 0.0f;
	float m_Xmax = 100.0f;
	float m_XOriginmin = 0.0f;
	float m_XOriginmax = 100.0f;
	float m_Ymin = 0;
	float m_Ymax = 100.0f;
	float m_YOriginmin = 0;
	float m_YOriginmax = 100.0f;
	float m_XOpenGLmin = -0.9f;
	float m_XOpenGLmax = 0.9f;
	float m_YOpenGLmin = -0.9f;
	float m_YOpenGLmax = 0.9f;
	int m_iXInterCnt = 10;
	int m_iYInterCnt = 10;
	bool m_bIsDrawXGrid = false;
	bool m_bIsDrawYGrid = false;
	float m_fMapScale;
	float m_fW2X;
	float m_fH2Y;
	QString m_strfightTime;
	QMap<QString, myline> m_mapLines;
	QMap<int, myEntity> m_mapEntitys;
	QSet<int> m_setHoverEntitys;
	QSet<int> m_setPressEntitys;
};

