#ifndef CONTROL_ENTITY_H
#define CONTROL_ENTITY_H

#include <string>
#include <vector>
#include <algorithm>
#include <curvedani.h>
#include "common/entity.h"
using std::find;

class iControlEntity : public Entity::iEntity
{
public:
    friend class Controller;
    iControlEntity(Entity &e) : Entity::iEntity(e), controller(0)
    {
    }
    virtual ~iControlEntity()
    {
        if (this->controller)
            dettach();
    }

    void attach(Controller &controller);
    void dettach();
protected:
    virtual void onControllerCommand(int command, const void *pData) = 0;
    
    Controller *controller;
};

class Controller
{
public:
    virtual void sendCommand(int command, const void *data = 0) = 0;
    virtual void attach(iControlEntity &entity) = 0;
    virtual void dettach(iControlEntity &entity) = 0;
protected:
    static void sendCommand(iControlEntity &entity, int command, const void *data)
    {
        entity.onControllerCommand(command, data);
    }
    static const Controller *getController(const iControlEntity &entity)
    {
        return entity.controller;
    }
    static void setController(iControlEntity &entity, Controller *controller)
    {
        entity.controller = controller;
    }
};

class SingleController : public Controller
{
public:
    SingleController() : entity(0)
    {
    }
    void sendCommand(int command, const void *data = 0)
    {
        if (entity)
            Controller::sendCommand(*entity, command, data);
    }
    void attach(iControlEntity &entity)
    {
        assert(getController(entity) == 0);
        if (this->entity == 0)
            this->entity = &entity;
        setController(entity, this);
    }
    void dettach(iControlEntity &entity)
    {
        assert(getController(entity) == this);
        setController(entity, 0);
        this->entity = 0;
    }
    bool isAttached() const
    {
        return entity != 0;
    }
protected:
    iControlEntity* entity;
};

class MultiController : public Controller
{
public:
    void sendCommand(int command, const void *data = 0)
    {
        for (vector<iControlEntity*>::iterator ie = entities.begin(); ie != entities.end(); ++ie)
            Controller::sendCommand(**ie, command, data);
    }
    void attach(iControlEntity &entity)
    {
        assert(getController(entity) == 0);
        if (find(entities.begin(), entities.end(), &entity) != entities.end())
        {
            assert(0);
            return;
        }
        entities.push_back(&entity);
        setController(entity, this);
    }
    void dettach(iControlEntity &entity)
    {
        assert(getController(entity) == this);
        vector<iControlEntity*>::iterator ie = find(entities.begin(), entities.end(), &entity);
        if (ie == entities.end())
        {
            assert(0);
            return;
        }
        setController(entity, 0);
        entities.erase(remove(entities.begin(), entities.end(), &entity), entities.end());
    }
    size_t getAttachedCount() const
    {
        return entities.size();
    }
protected:
    vector<iControlEntity*> entities;
};

inline
void iControlEntity::attach(Controller &controller)
{
    if (this->controller)
        dettach();
    controller.attach(*this);
    assert(this->controller == &controller);
}

inline
void iControlEntity::dettach()
{
    assert(this->controller);
    this->controller->dettach(*this);
    assert(this->controller == 0);
}
#endif