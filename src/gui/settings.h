/*!
  \file settings.h
  \brief Convenient settings (reused from assignment 1).
  \author Samuel Ng
  \date 2021-01-13 created
  \copyright GNU LGPL v3
  */

#ifndef _SETTINGS_H_
#define _SETTINGS_H_

#include <QObject>
#include <QColor>

namespace st {

  //! Class containing handy settings.
  class Settings
  {
  public:

    //! Decision tree scaling factor.
    static qreal sf;

    //! Graphics viewer scaling factor (how many pixels per grid cell).
    static qreal sf_grid;

    //! \brief Return a generated color.
    //!
    //! Return a color that generated as suitable for the provided index and 
    //! max possible index.
    static QColor colorGenerator(int ind, int max_ind);

    //! Generated colors.
    static QList<QColor> gcols;

    //! Count of colors the current gcols is for.
    static int gcols_for;

  };

}


#endif
