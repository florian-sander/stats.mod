/*
 * Copyright (C) 2000,2001  Florian Sander
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#define SLYEAR          getdur(0)
#define SLYEARS         getdur(1)
#define SLWEEK          getdur(2)
#define SLWEEKS         getdur(3)
#define SLDAY           getdur(4)
#define SLDAYS          getdur(5)
#define SLHOUR          getdur(6)
#define SLHOURS         getdur(7)
#define SLMINUTE        getdur(8)
#define SLMINUTES       getdur(9)
#define SLSECOND        getdur(10)
#define SLSECONDS       getdur(11)
#define SLSOMETIME	getdur(12)

#define SLTOTAL         getslang(110)
#define SLDAILY         getslang(111)
#define SLWEEKLY              getslang(112)
#define SLMONTHLY             getslang(113)

#define SLNOSTATSABOUTSOMEONE	getslang(220)
#define SLNOSTATSABOUTYOU	getslang(221)

#define SLUSERSMOSTUSEDWORDS	getslang(300)
#define SLNOWORDSTATS	getslang(310)
#define SLCHANSMOSTUSEDWORDS	getslang(320)
#define SLNOCHANWORDSTATS	getslang(330)

#define SLLTOTAL        getslang(750)
#define SLLTODAY        getslang(751)
#define SLLWEEKLY       getslang(752)
#define SLLMONTHLY      getslang(753)

#define SLDONTRECOGNIZE getslang(1020)

#define SLPASSALREADYSET getslang(1100)
#define SLPASSUSAGE      getslang(1110)
#define SLPASSSET        getslang(1120)

#define SL_EMAILPASS_SUBJECT	getslang(1200)
#define	SL_EMAILPASS_BODY		getslang(1210)


#ifdef __AUSKOMMENTIERT__
#define SLCSS		getslang(1)
#define SLBODYTAG       getslang(5)
#define SLHEADER        getslang(10)
#define SLFOOTER        getslang(11)

#define ROOTTITLE	getslang(100)
#define SLTOP		getslang(105)

#define SLUSERS		getslang(118)
#define SLONCHAN        getslang(119)
#define SLMISCSTATS	getslang(120)

#define SLINDEXTITEL	getslang(130)

#define SLTTOPTITLE	getslang(200)
#define SLDTOPTITLE	getslang(201)
#define SLWTOPTITLE	getslang(202)
#define SLMTOPTITLE	getslang(203)
#define SLTTOPHEAD	getslang(210)
#define SLDTOPHEAD	getslang(211)
#define SLWTOPHEAD	getslang(212)
#define SLMTOPHEAD	getslang(213)
#define SLORDEREDBY	getslang(214)
#define SLPEAK		getslang(215)
#define SLTNR           getslang(216)
#define SLTOTALUSERS	getslang(217)
#define SLGSTARTED	getslang(218)
#define SLGRAPHS	      getslang(220)
#define SLOTHERCHANS	getslang(221)


#define SLGRTTITLE	getslang(300)
#define SLGRDTITLE	getslang(301)
#define SLGRWTITLE	getslang(302)
#define SLGRMTITLE	getslang(303)
#define SLGRTHEAD	      getslang(310)
#define SLGRDHEAD	      getslang(311)
#define SLGRWHEAD	      getslang(312)
#define SLGRMHEAD	      getslang(313)
#define SLGRORDEREDBY	getslang(320)
#define SLGRTOTAL	      getslang(321)
#define SLGROTHERS	getslang(322)
#define SLGRTABLE	      getslang(323)

#define SLUSERSTITLE	getslang(400)
#define SLUSERSONCHANTITLE      getslang(410)
#define SLNOWONCHAN	getslang(420)
#define SLIDLETIME	getslang(430)
#define SLNETSPLITTED   getslang(435)
#define SLITSME         getslang(436)

#define SLEMAIL		getslang(450)
#define SLHOMEPAGE	getslang(451)

#define SLUSERTITLE	getslang(500)
#define SLUSERHEAD	getslang(501)
#define SLPLACE getslang(502)
#define SLRANDQUOTE	getslang(510)
#define SLUWORDSTATS	getslang(520)
#define SLOTHERUSERS	getslang(530)

#define SLMISCTITLE	getslang(600)
#define SLMISCHEAD	getslang(610)
#define SLMAUSERS       getslang(620)
#define SLMTOPICS	      getslang(630)
#define SLMTOPICBY     	getslang(631)
#define SLMURLS         getslang(640)
#define SLMURLBY        getslang(641)
#define SLMMOSTUSED	getslang(650)
#define SLMISPS		getslang(651)
#define SLMTLDS		getslang(652)
#define SLMKICKS        getslang(660)
#define SLMCWORDSTATS	getslang(670)
#define SLMMOSTUSEDWORDS	getslang(671)

#define SLSTATICTITLE   getslang(900)
#define SLSTATICBODY    getslang(901)

#define SLNOSUCHTYPE    getslang(1000)
#define SLTOPWORD       getslang(1005)

#endif
