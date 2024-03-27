/* Copyright (c) 2002-2012 Croteam Ltd.
This program is free software; you can redistribute it and/or modify
it under the terms of version 2 of the GNU General Public License as published by
the Free Software Foundation


This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA. */

#ifndef SE_INCL_MENU_STARTERSAF_H
#define SE_INCL_MENU_STARTERSAF_H
#ifdef PRAGMA_ONCE
  #pragma once
#endif


BOOL LSLoadSinglePlayer(const CTFileName &fnm);
BOOL LSLoadNetwork(const CTFileName &fnm);
BOOL LSLoadSplitScreen(const CTFileName &fnm);
BOOL LSLoadDemo(const CTFileName &fnm);
BOOL LSLoadPlayerModel(const CTFileName &fnm);
BOOL LSLoadControls(const CTFileName &fnm);
BOOL LSLoadAddon(const CTFileName &fnm);
BOOL LSLoadMod(const CTFileName &fnm);
BOOL LSLoadAdvanced(const CTFileName &fnm);
BOOL LSLoadCustom(const CTFileName &fnm);
BOOL LSLoadNetSettings(const CTFileName &fnm);
BOOL LSSaveAnyGame(const CTFileName &fnm);
BOOL LSSaveDemo(const CTFileName &fnm);
void StartDemoPlay(void);
void StartNetworkLoadGame(void);
void StartSplitScreenGameLoad(void);

#endif  /* include-once check. */
