#ifndef IMAPADAPTERFACTORY_H
#define IMAPADAPTERFACTORY_H

#include "IMapAdapter.h"

class IMapAdapterFactory
{
public:
    //! Creates an instance of the actual plugin
    /*!
     * @return  a pointer to the MapAdapter
     */
    virtual IMapAdapter* CreateInstance() = 0;

    //! returns the unique identifier (Uuid) of this MapAdapter
    /*!
     * @return  the unique identifier (Uuid) of this MapAdapter
     */
    virtual QUuid	getId		() const = 0;

    //! returns the name of this MapAdapter
    /*!
     * @return  the name of this MapAdapter
     */
    virtual QString	getName		() const = 0;
};


Q_DECLARE_INTERFACE ( IMapAdapterFactory,
                      "com.cbsoft.Merkaartor.IMapAdapterFactory/1.0" )

#endif // IMAPADAPTERFACTORY_H
