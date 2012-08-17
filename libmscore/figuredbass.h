//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id: figuredbass.h 5526 2012-04-09 10:17:11Z lvinken $
//
//  Copyright (C) 2002-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __FIGUREDBASS_H__
#define __FIGUREDBASS_H__

#include "segment.h"
#include "text.h"

/*---------------------------------------------------------
NOTE ON ARCHITECTURE

FiguredBass elements are stored in the annotations of a Segment (like for instance Harmony)

FiguredBass is rather simple: it contains only _ticks, telling the duration of the element,
and a list of FiguredBassItem elements which do most of the job. It also maintains a text with the
normalized (made uniform) version of the text, which is used during editing.

Normally, a FiguredBass element is assumed to be styled with an internally maintained text style
(based on the parameters of the general style "Figured Bass") FIGURED_BASS style and it is set
in this way upon creation and upon layout().
- - - -
FiguredBassItem contains the actually f.b. info; it is made of 4 parts (in this order):
1) prefix: one of [nothing, doubleflat, flat, natural, sharp, doublesharp]
2) digit: one digit from 1 to 9
3) suffix: one of [nothing, doubleflat, flat, natural, sharp, doublesharp, plus, backslash, slash]
4) contLine: true if the item has a continuation line (whose length is determined by parent's _ticks)
and 5 parenthesis flags, one for each position before, between and after the four parts above:
each of them may contain one of [nothing, roundOpen, roundClosed, squaredOpen, squaredClosed].

There is a number of restrictions, implemented at the end of FiguredBassItem::parse().
Currently, no attempt is made to ensure that, if multiple parentheses are present, they are consistent
(matching open and closed parentheses is left to the user).

If an item cannot be parsed, the whole FiguredBass element is kept as entered, possibly un-styled.
If all items can be parsed, each item generates a display text from its properties,
lays it out so that it properly aligns under the chord, draws it at its proper location
and provides its FiguredBass parent with a normalized text for future editing.

FiguredBassItem has not use for formats (italics, bold, ...) and it is never edited directly;
more generally, it is never accessed directly, only via its FiguredBass parent;
so it is directly derived from Element and returns INVALID as type.

FiguredBass might require formatting (discouraged, but might be necessary for very uncommon cases)
and it is edited (via the normalized text); so it is derived from Text.
---------------------------------------------------------*/

//---------------------------------------------------------
//   @@ FiguredBassItem
///   One line of a figured bass indication
//
//    @P prefix               enum FiguredBassItem.ModifierNone, .ModifierDoubleFlat, .ModifierFlat, .ModifierNatural, .ModifierSharp, .ModifierDoubleSharp the accidental before the digit
//    @P digit                int         the main digit (0 - 9)
//    @P suffix               enum FiguredBassItem.ModifierNone, .ModifierDoubleFlat, .ModifierFlat, .ModifierNatural, .ModifierSharp, .ModifierDoubleSharp, .ModifierPlus, .ModifierBackslash, .ModifierSlash    the accidental/diacritic after the digit
//    @P continuationLine     bool        whether the item has a continuation line or not
//    @P parenthesis1         enum FiguredBassItem.ParenthesisNone, .ParenthesisRoundOpen, ParenthesisRoundClosed, ParenthesisSquaredOpen, ParenthesisSquaredClosed     the parentesis before the prefix
//    @P parenthesis2         enum FiguredBassItem.ParenthesisNone, .ParenthesisRoundOpen, ParenthesisRoundClosed, ParenthesisSquaredOpen, ParenthesisSquaredClosed     the parentesis after the prefix / before the digit
//    @P parenthesis3         enum FiguredBassItem.ParenthesisNone, .ParenthesisRoundOpen, ParenthesisRoundClosed, ParenthesisSquaredOpen, ParenthesisSquaredClosed     the parentesis after the digit / before the suffix
//    @P parenthesis4         enum FiguredBassItem.ParenthesisNone, .ParenthesisRoundOpen, ParenthesisRoundClosed, ParenthesisSquaredOpen, ParenthesisSquaredClosed     the parentesis after the suffix / before the cont. line
//    @P parenthesis5         enum FiguredBassItem.ParenthesisNone, .ParenthesisRoundOpen, ParenthesisRoundClosed, ParenthesisSquaredOpen, ParenthesisSquaredClosed     the parentesis after the cont. line
//    @P displayText          string      R/O the text displayed (depends on configured fonts)
//    @P normalizedText       string      R/O conventional textual representation of item properties (= text used during input)
//---------------------------------------------------------

class FiguredBass;

class FiguredBassItem : public Element {
      Q_OBJECT
      Q_ENUMS(Modifier)
      Q_ENUMS(Parenthesis)
      Q_PROPERTY(Modifier     prefix            READ prefix       WRITE undoSetPrefix)
      Q_PROPERTY(int          digit             READ digit        WRITE undoSetDigit)
      Q_PROPERTY(Modifier     suffix            READ suffix       WRITE undoSetSuffix)
      Q_PROPERTY(bool         continuationLine  READ contLine     WRITE undoSetContLine)
      Q_PROPERTY(Parenthesis  parenthesis1      READ parenth1     WRITE undoSetParenth1)
      Q_PROPERTY(Parenthesis  parenthesis2      READ parenth2     WRITE undoSetParenth2)
      Q_PROPERTY(Parenthesis  parenthesis3      READ parenth3     WRITE undoSetParenth3)
      Q_PROPERTY(Parenthesis  parenthesis4      READ parenth4     WRITE undoSetParenth4)
      Q_PROPERTY(Parenthesis  parenthesis5      READ parenth5     WRITE undoSetParenth5)
      Q_PROPERTY(QString      displayText       READ displayText)
      Q_PROPERTY(QString      normalizedText    READ normalizedText)

   public:
      enum Modifier {
            ModifierNone = 0,
            ModifierDoubleFlat,
            ModifierFlat,
            ModifierNatural,
            ModifierSharp,
            ModifierDoubleSharp,
            ModifierPlus,
            ModifierBackslash,
            ModifierSlash,
                  NumOfModifiers
      };
      enum Parenthesis {
            ParenthesisNone = 0,
            ParenthesisRoundOpen,
            ParenthesisRoundClosed,
            ParenthesisSquaredOpen,
            ParenthesisSquaredClosed,
                  NumOfParentheses
      };

   private:

      static const QChar normParenthToChar[NumOfParentheses];

      QString           _displayText;           // the constructed display text (read-only)
      int               ord;                    // the line ordinal of this element in the FB stack
      // the parts making a FiguredBassItem up
      Modifier          _prefix;                // the accidental coming before the body
      int               _digit;                 // the main digit (if present)
      Modifier          _suffix;                // the accidental coming after the body
      bool              _contLine;              // wether the item has continuation line or not
      Parenthesis       parenth[5];             // each of the parenthesis: before, between and after parts
      qreal             textWidth;              // the text width (in raster units), set during layout()
                                                //    used by draw()
      // part parsing
      int               parseDigit(QString& str);
      int               parseParenthesis(QString& str, int parenthIdx);
      int               parsePrefixSuffix(QString& str, bool bPrefix);

      void              setDisplayText(const QString& s)    { _displayText = s;       }
      // read / write MusicXML support
      FiguredBassItem::Modifier MusicXML2Modifier(const QString prefix) const;
      QString                   Modifier2MusicXML(FiguredBassItem::Modifier prefix) const;

   public:
      FiguredBassItem(Score * s = 0, int line = 0);
      FiguredBassItem(const FiguredBassItem&);
      ~FiguredBassItem();

      FiguredBassItem &operator=(const FiguredBassItem&);

      // standard re-implemented virtual functions
      virtual FiguredBassItem*      clone() const     { return new FiguredBassItem(*this); }
      virtual ElementType           type() const      { return INVALID; }
      virtual void      draw(QPainter* painter) const;
      virtual void      layout();
      virtual void      read(const QDomElement&);
      virtual void      write(Xml& xml) const;

      // read / write MusicXML
      void              readMusicXML(const QDomElement& de, bool paren);
      void              writeMusicXML(Xml& xml) const;
      bool              startsWithParenthesis() const;

      // specific API
      const FiguredBass *    figuredBass() const      { return (FiguredBass*)(parent()); }
      bool              parse(QString& text);

      // getters / setters
      Modifier          prefix() const                { return _prefix;       }
      void              undoSetPrefix(Modifier pref);
      int               digit() const                 { return _digit;        }
      void              undoSetDigit(int digit);
      Modifier          suffix() const                { return _suffix;       }
      void              undoSetSuffix(Modifier suff);
      bool              contLine() const              { return _contLine;     }
      void              undoSetContLine(bool val);
      Parenthesis       parenth1()                    { return parenth[0];    }
      Parenthesis       parenth2()                    { return parenth[1];    }
      Parenthesis       parenth3()                    { return parenth[2];    }
      Parenthesis       parenth4()                    { return parenth[3];    }
      Parenthesis       parenth5()                    { return parenth[4];    }
      void              undoSetParenth1(Parenthesis par);
      void              undoSetParenth2(Parenthesis par);
      void              undoSetParenth3(Parenthesis par);
      void              undoSetParenth4(Parenthesis par);
      void              undoSetParenth5(Parenthesis par);
      QString           normalizedText() const;
      QString           displayText() const           { return _displayText;  }

      virtual QVariant  getProperty(P_ID propertyId) const;
      virtual bool      setProperty(P_ID propertyId, const QVariant&);
      virtual QVariant  propertyDefault(P_ID) const;
};

Q_DECLARE_METATYPE(FiguredBassItem::Modifier)
Q_DECLARE_METATYPE(FiguredBassItem::Parenthesis)

//---------------------------------------------------------
//   FiguredBassFont
//---------------------------------------------------------

struct FiguredBassFont {
      QString           family;
      QString           displayName;
      qreal             defPitch;
      qreal             defLineHeight;
      QChar             displayAccidental[6];
      QChar             displayParenthesis[5];
      QChar             displayDigit[2][10][4];

      bool read(const QDomElement&);
};

//---------------------------------------------------------
//   @@ FiguredBass
///   A complete figured bass indication
//
//    @P onNote   bool                    whether it is placed on a note beginning or between notes (r/o)
//    @P ticks    int                     duration in ticks
//---------------------------------------------------------

class FiguredBass : public Text {
      Q_OBJECT

//      Q_PROPERTY(QDeclarativeListProperty<FiguredBassItem> items READ qmlItems)
      Q_PROPERTY(bool   onNote      READ onNote)
      Q_PROPERTY(int    ticks       READ ticks  WRITE setTicks)

      QList<FiguredBassItem>  items;            // the individual lines of the F.B.
      QVector<qreal>    _lineLenghts;           // lengths of duration indicator lines (in raster units)
      bool              _onNote;                // true if this element is on a staff note | false if it is betweee notes
      int               _ticks;                 // the duration (used for cont. lines and for multiple F.B.
                                                // under the same note)
      void              layoutLines();
      bool              hasParentheses() const; // read / write MusicXML support

   public:
      FiguredBass(Score* s = 0);
      FiguredBass(const FiguredBass&);
      ~FiguredBass();

      // a convenience static function to create/retrieve a new FiguredBass into/from its intended parent
      static FiguredBass *    addFiguredBassToSegment(Segment *seg, int track, int extTicks, bool *pNew);

      // static functions for font config files
      static bool       readConfigFile(const QString& fileName);
      static QList<QString>  fontNames();
      static bool       fontData(int nIdx, QString *pFamily, QString *pDisplayName,
                              qreal * pSize, qreal * pLineHeight);

      // standard re-implemented virtual functions
      virtual FiguredBass*    clone() const     { return new FiguredBass(*this); }
      virtual ElementType     type() const      { return FIGURED_BASS; }
      virtual void      draw(QPainter* painter) const;
      virtual void      endEdit();
      virtual void      layout();
      virtual void      read(const QDomElement&);
      virtual void      setSelected(bool f);
      virtual void      setVisible(bool f);
      virtual void      write(Xml& xml) const;

      // read / write MusicXML
      void              readMusicXML(const QDomElement& de, int divisions);
      void              writeMusicXML(Xml& xml) const;

//DEBUG
Q_INVOKABLE FiguredBassItem* addItem();

      // getters / setters / properties
//      void qmlItemsAppend(QDeclarativeListProperty<FiguredBassItem> *list, FiguredBassItem * pItem)
//                                                {     list->append(pItem);
//                                                      items.append(&pItem);
//                                                }
//      QDeclarativeListProperty<FiguredBassItem> qmlItems()
//                                                {     QList<FiguredBassItem*> list;
//                                                      foreach(FiguredBassItem item, items)
//                                                            list.append(&item);
//                                                      return QDeclarativeListProperty<FiguredBassItem>(this, &items, qmlItemsAppend);
//                                                }
      qreal             lineLength(int idx) const     {   if(_lineLenghts.size() > idx)
                                                            return _lineLenghts.at(idx);
                                                          return 0;   }
      bool              onNote() const          { return _onNote; }
      void              setOnNote(bool val)     { _onNote = val;  }
      Segment *         segment() const         { return static_cast<Segment*>(parent()); }
      int               ticks() const           { return _ticks;  }
      void              setTicks(int val)       { _ticks = val;   }

      virtual QVariant  getProperty(P_ID propertyId) const;
      virtual bool      setProperty(P_ID propertyId, const QVariant&);
      virtual QVariant  propertyDefault(P_ID) const;
      };

#endif

