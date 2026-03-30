#ifndef STRUCTUREVIEW_H
#define STRUCTUREVIEW_H

#include <QGraphicsView>

class StructureView : public QGraphicsView
{
public:
    StructureView(QGraphicsView* parent = nullptr);
    ~StructureView();
};


#endif // STRUCTUREVIEW_H
