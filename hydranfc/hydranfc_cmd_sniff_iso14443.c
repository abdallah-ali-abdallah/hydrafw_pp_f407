/*
 * HydraBus/HydraNFC
 *
 * Copyright (C) 2012-2014 Benjamin VERNOUX
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "hydranfc_cmd_sniff_iso14443.h"

/*
 *  Detection of protocol with Start Bit and take into account noise in signal.
 *
 *  ISO14443-A Miller Modified@~106Khz Start bit:
 *   0x1F/31, 0x3F/63, 0x9F/159
 *
 *  ISO14443-A Manchester the subcarrier shall be 847.5KHz so 2x"1" then 2x"0" => 3.39/4 => Freq 847.5KHz
 *  00110011 00110011 00000000 00000000 = 0x33330000 or 0101 0000 = 0x50/0x70/0xA0/0xB0
 *  01110111 01110111 00000000 00000000 = 0x77770000 or 1111 0000 = 0xF0
 *  00110011 00110000 00000000 00000000 = 0x33300000 or 1110 0000 = 0xE0 / 0xE1
 *  01110111 01110000 00000000 00000000 = 0x77700000
 *  0x50/80, 0x70/112, 0xA0/160, 0xA1/161, 0xB0/176, 0xE0/224, 0xE1/225, 0xF0/240, 0xF1/241
 *
 *  Return 0 = Unknown Protocol
 *  Return 1 = ISO14443-A Miller Modified@~106Khz
 *  Return 2 = ISO14443-A Manchester@~106Khz
 *  other cases TBD (ISO14443-B ...)
 **/
const u08_t detected_protocol[256] = {
	0, /* v0 0000 0000 */
	0, /* v1 0000 0001 */
	0, /* v2 0000 0010 */
	0, /* v3 0000 0011 */
	0, /* v4 0000 0100 */
	0, /* v5 0000 0101 */
	0, /* v6 0000 0110 */
	0, /* v7 0000 0111 */
	0, /* v8  0000 1000 */
	0, /* v9  0000 1001 */
	0, /* v10 0000 1010 */
	0, /* v11 0000 1011 */
	0, /* v12 0000 1100 */
	0, /* v13 0000 1101 */
	0, /* v14 0000 1110 */
	0, /* v15 0000 1111 */

	0, /* v16 0001 0000 */
	0, /* v17 0001 0001 */
	0, /* v18 0001 0010 */
	0, /* v19 0001 0011 */
	0, /* v20 0001 0100 */
	0, /* v21 0001 0101 */
	0, /* v22 0001 0110 */
	0, /* v23 0001 0111 */
	0, /* v24 0001 1000 */
	0, /* v25 0001 1001 */
	0, /* v26 0001 1010 */
	0, /* v27 0001 1011 */
	0, /* v28 0001 1100 */
	0, /* v29 0001 1101 */
	0, /* v30 0001 1110 */
	MILLER_MODIFIED_106KHZ, /* v31 0001 1111 */

	0, /* v32 0010 0000 */
	0, /* v33 0010 0001 */
	0, /* v34 0010 0010 */
	0, /* v35 0010 0011 */
	0, /* v36 0010 0100 */
	0, /* v37 0010 0101 */
	0, /* v38 0010 0110 */
	0, /* v39 0010 0111 */
	0, /* v40 0010 1000 */
	0, /* v41 0010 1001 */
	0, /* v42 0010 1010 */
	0, /* v43 0010 1011 */
	0, /* v44 0010 1100 */
	0, /* v45 0010 1101 */
	0, /* v46 0010 1110 */
	0, /* v47 0010 1111 */

	0, /* v48 0011 0000 */
	0, /* v49 0011 0001 */
	0, /* v50 0011 0010 */
	0, /* v51 0011 0011 */
	0, /* v52 0011 0100 */
	0, /* v53 0011 0101 */
	0, /* v54 0011 0110 */
	0, /* v55 0011 0111 */
	0, /* v56 0011 1000 */
	0, /* v57 0011 1001 */
	0, /* v58 0011 1010 */
	0, /* v59 0011 1011 */
	0, /* v60 0011 1100 */
	0, /* v61 0011 1101 */
	0, /* v62 0011 1110 */
	MILLER_MODIFIED_106KHZ, /* v63 0011 1111 */

	0, /* v64 0100 0000 */
	0, /* v65 0100 0001 */
	0, /* v66 0100 0010 */
	0, /* v67 0100 0011 */
	0, /* v68 0100 0100 */
	0, /* v69 0100 0101 */
	0, /* v70 0100 0110 */
	0, /* v71 0100 0111 */
	0, /* v72 0100 1000 */
	0, /* v73 0100 1001 */
	0, /* v74 0100 1010 */
	0, /* v75 0100 1011 */
	0, /* v76 0100 1100 */
	0, /* v77 0100 1101 */
	0, /* v78 0100 1110 */
	0, /* v79 0100 1111 */

	MANCHESTER_106KHZ, /* v80 0101 0000 */
	0, /* v81 0101 0001 */
	0, /* v82 0101 0010 */
	0, /* v83 0101 0011 */
	0, /* v84 0101 0100 */
	0, /* v85 0101 0101 */
	0, /* v86 0101 0110 */
	0, /* v87 0101 0111 */
	0, /* v88 0101 1000 */
	0, /* v89 0101 1001 */
	0, /* v90 0101 1010 */
	0, /* v91 0101 1011 */
	0, /* v92 0101 1100 */
	0, /* v93 0101 1101 */
	0, /* v94 0101 1110 */
	0, /* v95 0101 1111 */

	0, /* v96  0110 0000 */
	0, /* v97  0110 0001 */
	0, /* v98  0110 0010 */
	0, /* v99  0110 0011 */
	0, /* v100 0110 0100 */
	0, /* v101 0110 0101 */
	0, /* v102 0110 0110 */
	0, /* v103 0110 0111 */
	0, /* v104 0110 1000 */
	0, /* v105 0110 1001 */
	0, /* v106 0110 1010 */
	0, /* v107 0110 1011 */
	0, /* v108 0110 1100 */
	0, /* v109 0110 1101 */
	0, /* v110 0110 1110 */
	0, /* v111 0110 1111 */

	MANCHESTER_106KHZ, /* v112 0111 0000 */
	0, /* v113 0111 0001 */
	0, /* v114 0111 0010 */
	0, /* v115 0111 0011 */
	0, /* v116 0111 0100 */
	0, /* v117 0111 0101 */
	0, /* v118 0111 0110 */
	0, /* v119 0111 0111 */
	0, /* v120 0111 1000 */
	0, /* v121 0111 1001 */
	0, /* v122 0111 1010 */
	0, /* v123 0111 1011 */
	0, /* v124 0111 1100 */
	0, /* v125 0111 1101 */
	0, /* v126 0111 1110 */
	0, /* v127 0111 1111 */

	0, /* v128 1000 0000 */
	0, /* v129 1000 0001 */
	0, /* v130 1000 0010 */
	0, /* v131 1000 0011 */
	0, /* v132 1000 0100 */
	0, /* v133 1000 0101 */
	0, /* v134 1000 0110 */
	0, /* v135 1000 0111 */
	0, /* v136 1000 1000 */
	0, /* v137 1000 1001 */
	0, /* v138 1000 1010 */
	0, /* v139 1000 1011 */
	0, /* v140 1000 1100 */
	0, /* v141 1000 1101 */
	0, /* v142 1000 1110 */
	0, /* v143 1000 1111 */

	0, /* v144 1001 0000 */
	0, /* v145 1001 0001 */
	0, /* v146 1001 0010 */
	0, /* v147 1001 0011 */
	0, /* v148 1001 0100 */
	0, /* v149 1001 0101 */
	0, /* v150 1001 0110 */
	0, /* v151 1001 0111 */
	0, /* v152 1001 1000 */
	0, /* v153 1001 1001 */
	0, /* v154 1001 1010 */
	0, /* v155 1001 1011 */
	0, /* v156 1001 1100 */
	0, /* v157 1001 1101 */
	0, /* v158 1001 1110 */
	MILLER_MODIFIED_106KHZ, /* v159 1001 1111 */

	MANCHESTER_106KHZ, /* v160 1010 0000 */
	MANCHESTER_106KHZ, /* v161 1010 0001 */
	0, /* v162 1010 0010 */
	0, /* v163 1010 0011 */
	0, /* v164 1010 0100 */
	0, /* v165 1010 0101 */
	0, /* v166 1010 0110 */
	0, /* v167 1010 0111 */
	0, /* v168 1010 1000 */
	0, /* v169 1010 1001 */
	0, /* v170 1010 1010 */
	0, /* v171 1010 1011 */
	0, /* v172 1010 1100 */
	0, /* v173 1010 1101 */
	0, /* v174 1010 1110 */
	0, /* v175 1010 1111 */

	MANCHESTER_106KHZ, /* v176 1011 0000 */
	0, /* v177 1011 0001 */
	0, /* v178 1011 0010 */
	0, /* v179 1011 0011 */
	0, /* v180 1011 0100 */
	0, /* v181 1011 0101 */
	0, /* v182 1011 0110 */
	0, /* v183 1011 0111 */
	0, /* v184 1011 1000 */
	0, /* v185 1011 1001 */
	0, /* v186 1011 1010 */
	0, /* v187 1011 1011 */
	0, /* v188 1011 1100 */
	0, /* v189 1011 1101 */
	0, /* v190 1011 1110 */
	0, /* v191 1011 1111 */

	0, /* v192 1100 0000 */
	0, /* v193 1100 0001 */
	0, /* v194 1100 0010 */
	0, /* v195 1100 0011 */
	0, /* v196 1100 0100 */
	0, /* v197 1100 0101 */
	0, /* v198 1100 0110 */
	0, /* v199 1100 0111 */
	0, /* v200 1100 1000 */
	0, /* v201 1100 1001 */
	0, /* v202 1100 1010 */
	0, /* v203 1100 1011 */
	0, /* v204 1100 1100 */
	0, /* v205 1100 1101 */
	0, /* v206 1100 1110 */
	0, /* v207 1100 1111 */

	0, /* v208 1101 0000 */
	0, /* v209 1101 0001 */
	0, /* v210 1101 0010 */
	0, /* v211 1101 0011 */
	0, /* v212 1101 0100 */
	0, /* v213 1101 0101 */
	0, /* v214 1101 0110 */
	0, /* v215 1101 0111 */
	0, /* v216 1101 1000 */
	0, /* v217 1101 1001 */
	0, /* v218 1101 1010 */
	0, /* v219 1101 1011 */
	0, /* v220 1101 1100 */
	0, /* v221 1101 1101 */
	0, /* v222 1101 1110 */
	0, /* v223 1101 1111 */

	MANCHESTER_106KHZ, /* v224 1110 0000 */
	MANCHESTER_106KHZ, /* v225 1110 0001 */
	0, /* v226 1110 0010 */
	0, /* v227 1110 0011 */
	0, /* v228 1110 0100 */
	0, /* v229 1110 0101 */
	0, /* v230 1110 0110 */
	0, /* v231 1110 0111 */
	0, /* v232 1110 1000 */
	0, /* v233 1110 1001 */
	0, /* v234 1110 1010 */
	0, /* v235 1110 1011 */
	0, /* v236 1110 1100 */
	0, /* v237 1110 1101 */
	0, /* v238 1110 1110 */
	0, /* v239 1110 1111 */

	MANCHESTER_106KHZ, /* v240 1111 0000 */
	MANCHESTER_106KHZ, /* v241 1111 0001 */
	0, /* v242 1111 0010 */
	0, /* v243 1111 0011 */
	0, /* v244 1111 0100 */
	0, /* v245 1111 0101 */
	0, /* v246 1111 0110 */
	0, /* v247 1111 0111 */
	0, /* v248 1111 1000 */
	0, /* v249 1111 1001 */
	0, /* v250 1111 1010 */
	0, /* v251 1111 1011 */
	0, /* v252 1111 1100 */
	0, /* v253 1111 1101 */
	0, /* v254 1111 1110 */
	0  /* v255 1111 1111 */
};


/* Miller Modified on 8bits, 8 bit In => 1 bit Out
 * 0xF1/0xF2/0xF3/0xF8/0xF9/0xFC => '1'
 * others = '0'
*/
const u08_t miller_modified_106kb[256]= {
	0, /* v0 0000 0000 */
	0, /* v1 0000 0001 */
	0, /* v2 0000 0010 */
	0, /* v3 0000 0011 */
	0, /* v4 0000 0100 */
	0, /* v5 0000 0101 */
	0, /* v6 0000 0110 */
	0, /* v7 0000 0111 */
	0, /* v8  0000 1000 */
	0, /* v9  0000 1001 */
	0, /* v10 0000 1010 */
	0, /* v11 0000 1011 */
	0, /* v12 0000 1100 */
	0, /* v13 0000 1101 */
	0, /* v14 0000 1110 */
	0, /* v15 0000 1111 */

	0, /* v16 0001 0000 */
	0, /* v17 0001 0001 */
	0, /* v18 0001 0010 */
	0, /* v19 0001 0011 */
	0, /* v20 0001 0100 */
	0, /* v21 0001 0101 */
	0, /* v22 0001 0110 */
	0, /* v23 0001 0111 */
	0, /* v24 0001 1000 */
	0, /* v25 0001 1001 */
	0, /* v26 0001 1010 */
	0, /* v27 0001 1011 */
	0, /* v28 0001 1100 */
	0, /* v29 0001 1101 */
	0, /* v30 0001 1110 */
	0, /* v31 0001 1111 */

	0, /* v32 0010 0000 */
	0, /* v33 0010 0001 */
	0, /* v34 0010 0010 */
	0, /* v35 0010 0011 */
	0, /* v36 0010 0100 */
	0, /* v37 0010 0101 */
	0, /* v38 0010 0110 */
	0, /* v39 0010 0111 */
	0, /* v40 0010 1000 */
	0, /* v41 0010 1001 */
	0, /* v42 0010 1010 */
	0, /* v43 0010 1011 */
	0, /* v44 0010 1100 */
	0, /* v45 0010 1101 */
	0, /* v46 0010 1110 */
	0, /* v47 0010 1111 */

	0, /* v48 0011 0000 */
	0, /* v49 0011 0001 */
	0, /* v50 0011 0010 */
	0, /* v51 0011 0011 */
	0, /* v52 0011 0100 */
	0, /* v53 0011 0101 */
	0, /* v54 0011 0110 */
	0, /* v55 0011 0111 */
	0, /* v56 0011 1000 */
	0, /* v57 0011 1001 */
	0, /* v58 0011 1010 */
	0, /* v59 0011 1011 */
	0, /* v60 0011 1100 */
	0, /* v61 0011 1101 */
	0, /* v62 0011 1110 */
	0, /* v63 0011 1111 */

	0, /* v64 0100 0000 */
	0, /* v65 0100 0001 */
	0, /* v66 0100 0010 */
	0, /* v67 0100 0011 */
	0, /* v68 0100 0100 */
	0, /* v69 0100 0101 */
	0, /* v70 0100 0110 */
	0, /* v71 0100 0111 */
	0, /* v72 0100 1000 */
	0, /* v73 0100 1001 */
	0, /* v74 0100 1010 */
	0, /* v75 0100 1011 */
	0, /* v76 0100 1100 */
	0, /* v77 0100 1101 */
	0, /* v78 0100 1110 */
	0, /* v79 0100 1111 */

	0, /* v80 0101 0000 */
	0, /* v81 0101 0001 */
	0, /* v82 0101 0010 */
	0, /* v83 0101 0011 */
	0, /* v84 0101 0100 */
	0, /* v85 0101 0101 */
	0, /* v86 0101 0110 */
	0, /* v87 0101 0111 */
	0, /* v88 0101 1000 */
	0, /* v89 0101 1001 */
	0, /* v90 0101 1010 */
	0, /* v91 0101 1011 */
	0, /* v92 0101 1100 */
	0, /* v93 0101 1101 */
	0, /* v94 0101 1110 */
	0, /* v95 0101 1111 */

	0, /* v96  0110 0000 */
	0, /* v97  0110 0001 */
	0, /* v98  0110 0010 */
	0, /* v99  0110 0011 */
	0, /* v100 0110 0100 */
	0, /* v101 0110 0101 */
	0, /* v102 0110 0110 */
	0, /* v103 0110 0111 */
	0, /* v104 0110 1000 */
	0, /* v105 0110 1001 */
	0, /* v106 0110 1010 */
	0, /* v107 0110 1011 */
	0, /* v108 0110 1100 */
	0, /* v109 0110 1101 */
	0, /* v110 0110 1110 */
	0, /* v111 0110 1111 */

	0, /* v112 0111 0000 */
	0, /* v113 0111 0001 */
	0, /* v114 0111 0010 */
	0, /* v115 0111 0011 */
	0, /* v116 0111 0100 */
	0, /* v117 0111 0101 */
	0, /* v118 0111 0110 */
	0, /* v119 0111 0111 */
	0, /* v120 0111 1000 */
	0, /* v121 0111 1001 */
	0, /* v122 0111 1010 */
	0, /* v123 0111 1011 */
	0, /* v124 0111 1100 */
	0, /* v125 0111 1101 */
	0, /* v126 0111 1110 */
	0, /* v127 0111 1111 */

	0, /* v128 1000 0000 */
	0, /* v129 1000 0001 */
	0, /* v130 1000 0010 */
	0, /* v131 1000 0011 */
	0, /* v132 1000 0100 */
	0, /* v133 1000 0101 */
	0, /* v134 1000 0110 */
	0, /* v135 1000 0111 */
	0, /* v136 1000 1000 */
	0, /* v137 1000 1001 */
	0, /* v138 1000 1010 */
	0, /* v139 1000 1011 */
	0, /* v140 1000 1100 */
	0, /* v141 1000 1101 */
	0, /* v142 1000 1110 */
	0, /* v143 1000 1111 */

	0, /* v144 1001 0000 */
	0, /* v145 1001 0001 */
	0, /* v146 1001 0010 */
	0, /* v147 1001 0011 */
	0, /* v148 1001 0100 */
	0, /* v149 1001 0101 */
	0, /* v150 1001 0110 */
	0, /* v151 1001 0111 */
	0, /* v152 1001 1000 */
	0, /* v153 1001 1001 */
	0, /* v154 1001 1010 */
	0, /* v155 1001 1011 */
	0, /* v156 1001 1100 */
	0, /* v157 1001 1101 */
	0, /* v158 1001 1110 */
	0, /* v159 1001 1111 */

	0, /* v160 1010 0000 */
	0, /* v161 1010 0001 */
	0, /* v162 1010 0010 */
	0, /* v163 1010 0011 */
	0, /* v164 1010 0100 */
	0, /* v165 1010 0101 */
	0, /* v166 1010 0110 */
	0, /* v167 1010 0111 */
	0, /* v168 1010 1000 */
	0, /* v169 1010 1001 */
	0, /* v170 1010 1010 */
	0, /* v171 1010 1011 */
	0, /* v172 1010 1100 */
	0, /* v173 1010 1101 */
	0, /* v174 1010 1110 */
	0, /* v175 1010 1111 */

	0, /* v176 1011 0000 */
	0, /* v177 1011 0001 */
	0, /* v178 1011 0010 */
	0, /* v179 1011 0011 */
	0, /* v180 1011 0100 */
	0, /* v181 1011 0101 */
	0, /* v182 1011 0110 */
	0, /* v183 1011 0111 */
	0, /* v184 1011 1000 */
	0, /* v185 1011 1001 */
	0, /* v186 1011 1010 */
	0, /* v187 1011 1011 */
	0, /* v188 1011 1100 */
	0, /* v189 1011 1101 */
	0, /* v190 1011 1110 */
	0, /* v191 1011 1111 */

	0, /* v192 1100 0000 */
	0, /* v193 1100 0001 */
	0, /* v194 1100 0010 */
	0, /* v195 1100 0011 */
	0, /* v196 1100 0100 */
	0, /* v197 1100 0101 */
	0, /* v198 1100 0110 */
	0, /* v199 1100 0111 */
	0, /* v200 1100 1000 */
	0, /* v201 1100 1001 */
	0, /* v202 1100 1010 */
	0, /* v203 1100 1011 */
	0, /* v204 1100 1100 */
	0, /* v205 1100 1101 */
	0, /* v206 1100 1110 */
	0, /* v207 1100 1111 */

	0, /* v208 1101 0000 */
	0, /* v209 1101 0001 */
	0, /* v210 1101 0010 */
	0, /* v211 1101 0011 */
	0, /* v212 1101 0100 */
	0, /* v213 1101 0101 */
	0, /* v214 1101 0110 */
	0, /* v215 1101 0111 */
	0, /* v216 1101 1000 */
	0, /* v217 1101 1001 */
	0, /* v218 1101 1010 */
	0, /* v219 1101 1011 */
	0, /* v220 1101 1100 */
	0, /* v221 1101 1101 */
	0, /* v222 1101 1110 */
	0, /* v223 1101 1111 */

	0, /* v224 1110 0000 */
	0, /* v225 1110 0001 */
	0, /* v226 1110 0010 */
	0, /* v227 1110 0011 */
	0, /* v228 1110 0100 */
	0, /* v229 1110 0101 */
	0, /* v230 1110 0110 */
	0, /* v231 1110 0111 */
	0, /* v232 1110 1000 */
	0, /* v233 1110 1001 */
	0, /* v234 1110 1010 */
	0, /* v235 1110 1011 */
	0, /* v236 1110 1100 */
	0, /* v237 1110 1101 */
	0, /* v238 1110 1110 */
	0, /* v239 1110 1111 */

	1, /* v240 1111 0000 */
	1, /* v241 1111 0001 */
	0, /* v242 1111 0010 */
	1, /* v243 1111 0011 */
	0, /* v244 1111 0100 */
	0, /* v245 1111 0101 */
	0, /* v246 1111 0110 */
	0, /* v247 1111 0111 */
	1, /* v248 1111 1000 */
	1, /* v249 1111 1001 */
	0, /* v250 1111 1010 */
	0, /* v251 1111 1011 */
	1, /* v252 1111 1100 */
	0, /* v253 1111 1101 */
	0, /* v254 1111 1110 */
	0  /* v255 1111 1111 */
};

/* Manchester 8 bit In => 1 bit Out
 * 0x50/0x70/0x90/0xA0/0xB0/0xC0/0xD0/0xE0/0xF0 = '1',
 * 0x51/0x71/0x91/0xA1/0xB1/0xC1/0xD1/0xE1/0xF1 = '1',
 * others = '0'
*/
const u08_t manchester_106kb[256]= {
	0, /* v0 0000 0000 */
	0, /* v1 0000 0001 */
	0, /* v2 0000 0010 */
	0, /* v3 0000 0011 */
	0, /* v4 0000 0100 */
	0, /* v5 0000 0101 */
	0, /* v6 0000 0110 */
	0, /* v7 0000 0111 */
	0, /* v8  0000 1000 */
	0, /* v9  0000 1001 */
	0, /* v10 0000 1010 */
	0, /* v11 0000 1011 */
	0, /* v12 0000 1100 */
	0, /* v13 0000 1101 */
	0, /* v14 0000 1110 */
	0, /* v15 0000 1111 */

	0, /* v16 0001 0000 */
	0, /* v17 0001 0001 */
	0, /* v18 0001 0010 */
	0, /* v19 0001 0011 */
	0, /* v20 0001 0100 */
	0, /* v21 0001 0101 */
	0, /* v22 0001 0110 */
	0, /* v23 0001 0111 */
	0, /* v24 0001 1000 */
	0, /* v25 0001 1001 */
	0, /* v26 0001 1010 */
	0, /* v27 0001 1011 */
	0, /* v28 0001 1100 */
	0, /* v29 0001 1101 */
	0, /* v30 0001 1110 */
	0, /* v31 0001 1111 */

	0, /* v32 0010 0000 */
	0, /* v33 0010 0001 */
	0, /* v34 0010 0010 */
	0, /* v35 0010 0011 */
	0, /* v36 0010 0100 */
	0, /* v37 0010 0101 */
	0, /* v38 0010 0110 */
	0, /* v39 0010 0111 */
	0, /* v40 0010 1000 */
	0, /* v41 0010 1001 */
	0, /* v42 0010 1010 */
	0, /* v43 0010 1011 */
	0, /* v44 0010 1100 */
	0, /* v45 0010 1101 */
	0, /* v46 0010 1110 */
	0, /* v47 0010 1111 */

	0, /* v48 0011 0000 */
	0, /* v49 0011 0001 */
	0, /* v50 0011 0010 */
	0, /* v51 0011 0011 */
	0, /* v52 0011 0100 */
	0, /* v53 0011 0101 */
	0, /* v54 0011 0110 */
	0, /* v55 0011 0111 */
	0, /* v56 0011 1000 */
	0, /* v57 0011 1001 */
	0, /* v58 0011 1010 */
	0, /* v59 0011 1011 */
	0, /* v60 0011 1100 */
	0, /* v61 0011 1101 */
	0, /* v62 0011 1110 */
	0, /* v63 0011 1111 */

	0, /* v64 0100 0000 */
	0, /* v65 0100 0001 */
	0, /* v66 0100 0010 */
	0, /* v67 0100 0011 */
	0, /* v68 0100 0100 */
	0, /* v69 0100 0101 */
	0, /* v70 0100 0110 */
	0, /* v71 0100 0111 */
	0, /* v72 0100 1000 */
	0, /* v73 0100 1001 */
	0, /* v74 0100 1010 */
	0, /* v75 0100 1011 */
	0, /* v76 0100 1100 */
	0, /* v77 0100 1101 */
	0, /* v78 0100 1110 */
	0, /* v79 0100 1111 */

	0, /* v80 0101 0000 */
	0, /* v81 0101 0001 */
	0, /* v82 0101 0010 */
	0, /* v83 0101 0011 */
	0, /* v84 0101 0100 */
	0, /* v85 0101 0101 */
	0, /* v86 0101 0110 */
	0, /* v87 0101 0111 */
	0, /* v88 0101 1000 */
	0, /* v89 0101 1001 */
	0, /* v90 0101 1010 */
	0, /* v91 0101 1011 */
	0, /* v92 0101 1100 */
	0, /* v93 0101 1101 */
	0, /* v94 0101 1110 */
	0, /* v95 0101 1111 */

	0, /* v96  0110 0000 */
	0, /* v97  0110 0001 */
	0, /* v98  0110 0010 */
	0, /* v99  0110 0011 */
	0, /* v100 0110 0100 */
	0, /* v101 0110 0101 */
	0, /* v102 0110 0110 */
	0, /* v103 0110 0111 */
	0, /* v104 0110 1000 */
	0, /* v105 0110 1001 */
	0, /* v106 0110 1010 */
	0, /* v107 0110 1011 */
	0, /* v108 0110 1100 */
	0, /* v109 0110 1101 */
	0, /* v110 0110 1110 */
	0, /* v111 0110 1111 */

	0, /* v112 0111 0000 */
	0, /* v113 0111 0001 */
	0, /* v114 0111 0010 */
	0, /* v115 0111 0011 */
	0, /* v116 0111 0100 */
	0, /* v117 0111 0101 */
	0, /* v118 0111 0110 */
	0, /* v119 0111 0111 */
	0, /* v120 0111 1000 */
	0, /* v121 0111 1001 */
	0, /* v122 0111 1010 */
	0, /* v123 0111 1011 */
	0, /* v124 0111 1100 */
	0, /* v125 0111 1101 */
	0, /* v126 0111 1110 */
	0, /* v127 0111 1111 */

	0, /* v128 1000 0000 */
	0, /* v129 1000 0001 */
	0, /* v130 1000 0010 */
	0, /* v131 1000 0011 */
	0, /* v132 1000 0100 */
	0, /* v133 1000 0101 */
	0, /* v134 1000 0110 */
	0, /* v135 1000 0111 */
	0, /* v136 1000 1000 */
	0, /* v137 1000 1001 */
	0, /* v138 1000 1010 */
	0, /* v139 1000 1011 */
	0, /* v140 1000 1100 */
	0, /* v141 1000 1101 */
	0, /* v142 1000 1110 */
	0, /* v143 1000 1111 */

	0, /* v144 1001 0000 */
	0, /* v145 1001 0001 */
	0, /* v146 1001 0010 */
	0, /* v147 1001 0011 */
	0, /* v148 1001 0100 */
	0, /* v149 1001 0101 */
	0, /* v150 1001 0110 */
	0, /* v151 1001 0111 */
	0, /* v152 1001 1000 */
	0, /* v153 1001 1001 */
	0, /* v154 1001 1010 */
	0, /* v155 1001 1011 */
	0, /* v156 1001 1100 */
	0, /* v157 1001 1101 */
	0, /* v158 1001 1110 */
	0, /* v159 1001 1111 */

	1, /* v160 1010 0000 */
	1, /* v161 1010 0001 */
	0, /* v162 1010 0010 */
	0, /* v163 1010 0011 */
	0, /* v164 1010 0100 */
	0, /* v165 1010 0101 */
	0, /* v166 1010 0110 */
	0, /* v167 1010 0111 */
	0, /* v168 1010 1000 */
	0, /* v169 1010 1001 */
	0, /* v170 1010 1010 */
	0, /* v171 1010 1011 */
	0, /* v172 1010 1100 */
	0, /* v173 1010 1101 */
	0, /* v174 1010 1110 */
	0, /* v175 1010 1111 */

	1, /* v176 1011 0000 */
	1, /* v177 1011 0001 */
	0, /* v178 1011 0010 */
	0, /* v179 1011 0011 */
	0, /* v180 1011 0100 */
	0, /* v181 1011 0101 */
	0, /* v182 1011 0110 */
	0, /* v183 1011 0111 */
	0, /* v184 1011 1000 */
	0, /* v185 1011 1001 */
	0, /* v186 1011 1010 */
	0, /* v187 1011 1011 */
	0, /* v188 1011 1100 */
	0, /* v189 1011 1101 */
	0, /* v190 1011 1110 */
	0, /* v191 1011 1111 */

	1, /* v192 1100 0000 */
	1, /* v193 1100 0001 */
	0, /* v194 1100 0010 */
	0, /* v195 1100 0011 */
	0, /* v196 1100 0100 */
	0, /* v197 1100 0101 */
	0, /* v198 1100 0110 */
	0, /* v199 1100 0111 */
	0, /* v200 1100 1000 */
	0, /* v201 1100 1001 */
	0, /* v202 1100 1010 */
	0, /* v203 1100 1011 */
	0, /* v204 1100 1100 */
	0, /* v205 1100 1101 */
	0, /* v206 1100 1110 */
	0, /* v207 1100 1111 */

	1, /* v208 1101 0000 */
	1, /* v209 1101 0001 */
	0, /* v210 1101 0010 */
	0, /* v211 1101 0011 */
	0, /* v212 1101 0100 */
	0, /* v213 1101 0101 */
	0, /* v214 1101 0110 */
	0, /* v215 1101 0111 */
	0, /* v216 1101 1000 */
	0, /* v217 1101 1001 */
	0, /* v218 1101 1010 */
	0, /* v219 1101 1011 */
	0, /* v220 1101 1100 */
	0, /* v221 1101 1101 */
	0, /* v222 1101 1110 */
	0, /* v223 1101 1111 */

	1, /* v224 1110 0000 */
	1, /* v225 1110 0001 */
	0, /* v226 1110 0010 */
	0, /* v227 1110 0011 */
	0, /* v228 1110 0100 */
	0, /* v229 1110 0101 */
	0, /* v230 1110 0110 */
	0, /* v231 1110 0111 */
	0, /* v232 1110 1000 */
	0, /* v233 1110 1001 */
	0, /* v234 1110 1010 */
	0, /* v235 1110 1011 */
	0, /* v236 1110 1100 */
	0, /* v237 1110 1101 */
	0, /* v238 1110 1110 */
	0, /* v239 1110 1111 */

	1, /* v240 1111 0000 */
	1, /* v241 1111 0001 */
	0, /* v242 1111 0010 */
	0, /* v243 1111 0011 */
	0, /* v244 1111 0100 */
	0, /* v245 1111 0101 */
	0, /* v246 1111 0110 */
	0, /* v247 1111 0111 */
	0, /* v248 1111 1000 */
	0, /* v249 1111 1001 */
	0, /* v250 1111 1010 */
	0, /* v251 1111 1011 */
	0, /* v252 1111 1100 */
	0, /* v253 1111 1101 */
	0, /* v254 1111 1110 */
	0  /* v255 1111 1111 */
};

