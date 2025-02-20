#pragma once

#include <algorithm>
#include <bit>
#include <cassert>
#include <cstdint>

template <uint32_t S> struct bs_table {
  constexpr bs_table() : data() {
    constexpr uint32_t s = std::bit_width(S) - 1;
    for (uint32_t blT = 0; blT < 32; ++blT) {
      const uint32_t t = blT - std::min(s, blT); // Current epoch
      const uint32_t blt = std::bit_width(t);    // Bit length of t

      bool epsilon_tau = std::bit_floor(t << 1) > t + blt; // Correction factor
      const uint32_t tau = blt - epsilon_tau;              // Current meta-epoch
      data[blT] = std::max<uint32_t>(S >> (tau + 1), 1);
      // ^^^ Num bunches available to h.v.
    }
  }
  uint32_t data[32];
};

template <uint32_t S> inline uint32_t lookup_bs(uint32_t x) {
  constexpr bs_table<S> bs{};
  switch (x) {
  case 0:
    return bs.data[0];
  case 1:
    return bs.data[1];
  case 2:
    return bs.data[2];
  case 3:
    return bs.data[3];
  case 4:
    return bs.data[4];
  case 5:
    return bs.data[5];
  case 6:
    return bs.data[6];
  case 7:
    return bs.data[7];
  case 8:
    return bs.data[8];
  case 9:
    return bs.data[9];
  case 10:
    return bs.data[10];
  case 11:
    return bs.data[11];
  case 12:
    return bs.data[12];
  case 13:
    return bs.data[13];
  case 14:
    return bs.data[14];
  case 15:
    return bs.data[15];
  case 16:
    return bs.data[16];
  case 17:
    return bs.data[17];
  case 18:
    return bs.data[18];
  case 19:
    return bs.data[19];
  case 20:
    return bs.data[20];
  case 21:
    return bs.data[21];
  case 22:
    return bs.data[22];
  case 23:
    return bs.data[23];
  case 24:
    return bs.data[24];
  case 25:
    return bs.data[25];
  case 26:
    return bs.data[26];
  case 27:
    return bs.data[27];
  case 28:
    return bs.data[28];
  case 29:
    return bs.data[29];
  case 30:
    return bs.data[30];
  case 31:
    return bs.data[31];
  default:
    __builtin_unreachable();
  }
}

template <uint32_t S>
uint32_t inline constexpr calc_B(const uint32_t blT, const uint32_t h) {
  const uint32_t s = std::bit_width(S) - 1;
  const uint32_t t = blT - std::min(s, blT); // Current epoch

  const uint32_t blt = std::bit_width(t); // Bit length of t
  bool epsilon_tau =
      std::bit_floor<uint32_t>(t << 1) > t + blt; // Correction factor
  const uint32_t tau = blt - epsilon_tau;         // Current meta-epoch
  const uint32_t t_0 = (1 << tau) - tau;          // Opening epoch of meta-epoch
  const uint32_t t_1 =
      (1 << (tau + 1)) - (tau + 1); // Opening epoch of next meta-epoch
  const bool epsilon_b =
      t < h + t_0 && h + t_0 < t_1; // Uninvaded correction factor
  const uint32_t B = std::max<uint32_t>(S >> (tau + 1 - epsilon_b), 1);
  // ^^^ Num bunches available to h.v.
  return B;
}

template <uint32_t S, uint32_t max_h> struct B_table {

  constexpr B_table() : data() {
    for (uint32_t h = 0; h < max_h; ++h) {
      for (uint32_t blT = 0; blT < 32; ++blT) {
        data[h * 32 + blT] = calc_B<S>(blT, h);
      }
    }
  }
  uint32_t data[32 * max_h];
};

template <uint32_t S, uint32_t max_h>
inline uint32_t lookup_B(const uint32_t blT, const uint32_t h) {
  static_assert(max_h <= 8);
  assert(h < max_h);
  uint32_t x = h * 32 + blT;
  constexpr B_table<S, max_h> table{};
  switch (x) {
  case 0:
    return table.data[0];
  case 1:
    return table.data[1];
  case 2:
    return table.data[2];
  case 3:
    return table.data[3];
  case 4:
    return table.data[4];
  case 5:
    return table.data[5];
  case 6:
    return table.data[6];
  case 7:
    return table.data[7];
  case 8:
    return table.data[8];
  case 9:
    return table.data[9];
  case 10:
    return table.data[10];
  case 11:
    return table.data[11];
  case 12:
    return table.data[12];
  case 13:
    return table.data[13];
  case 14:
    return table.data[14];
  case 15:
    return table.data[15];
  case 16:
    return table.data[16];
  case 17:
    return table.data[17];
  case 18:
    return table.data[18];
  case 19:
    return table.data[19];
  case 20:
    return table.data[20];
  case 21:
    return table.data[21];
  case 22:
    return table.data[22];
  case 23:
    return table.data[23];
  case 24:
    return table.data[24];
  case 25:
    return table.data[25];
  case 26:
    return table.data[26];
  case 27:
    return table.data[27];
  case 28:
    return table.data[28];
  case 29:
    return table.data[29];
  case 30:
    return table.data[30];
  case 31:
    return table.data[31];
  case 32:
    return table.data[32];
  case 33:
    return table.data[33];
  case 34:
    return table.data[34];
  case 35:
    return table.data[35];
  case 36:
    return table.data[36];
  case 37:
    return table.data[37];
  case 38:
    return table.data[38];
  case 39:
    return table.data[39];
  case 40:
    return table.data[40];
  case 41:
    return table.data[41];
  case 42:
    return table.data[42];
  case 43:
    return table.data[43];
  case 44:
    return table.data[44];
  case 45:
    return table.data[45];
  case 46:
    return table.data[46];
  case 47:
    return table.data[47];
  case 48:
    return table.data[48];
  case 49:
    return table.data[49];
  case 50:
    return table.data[50];
  case 51:
    return table.data[51];
  case 52:
    return table.data[52];
  case 53:
    return table.data[53];
  case 54:
    return table.data[54];
  case 55:
    return table.data[55];
  case 56:
    return table.data[56];
  case 57:
    return table.data[57];
  case 58:
    return table.data[58];
  case 59:
    return table.data[59];
  case 60:
    return table.data[60];
  case 61:
    return table.data[61];
  case 62:
    return table.data[62];
  case 63:
    return table.data[63];
  case 64:
    return table.data[64];
  case 65:
    return table.data[65];
  case 66:
    return table.data[66];
  case 67:
    return table.data[67];
  case 68:
    return table.data[68];
  case 69:
    return table.data[69];
  case 70:
    return table.data[70];
  case 71:
    return table.data[71];
  case 72:
    return table.data[72];
  case 73:
    return table.data[73];
  case 74:
    return table.data[74];
  case 75:
    return table.data[75];
  case 76:
    return table.data[76];
  case 77:
    return table.data[77];
  case 78:
    return table.data[78];
  case 79:
    return table.data[79];
  case 80:
    return table.data[80];
  case 81:
    return table.data[81];
  case 82:
    return table.data[82];
  case 83:
    return table.data[83];
  case 84:
    return table.data[84];
  case 85:
    return table.data[85];
  case 86:
    return table.data[86];
  case 87:
    return table.data[87];
  case 88:
    return table.data[88];
  case 89:
    return table.data[89];
  case 90:
    return table.data[90];
  case 91:
    return table.data[91];
  case 92:
    return table.data[92];
  case 93:
    return table.data[93];
  case 94:
    return table.data[94];
  case 95:
    return table.data[95];
  case 96:
    return table.data[96];
  case 97:
    return table.data[97];
  case 98:
    return table.data[98];
  case 99:
    return table.data[99];
  case 100:
    return table.data[100];
  case 101:
    return table.data[101];
  case 102:
    return table.data[102];
  case 103:
    return table.data[103];
  case 104:
    return table.data[104];
  case 105:
    return table.data[105];
  case 106:
    return table.data[106];
  case 107:
    return table.data[107];
  case 108:
    return table.data[108];
  case 109:
    return table.data[109];
  case 110:
    return table.data[110];
  case 111:
    return table.data[111];
  case 112:
    return table.data[112];
  case 113:
    return table.data[113];
  case 114:
    return table.data[114];
  case 115:
    return table.data[115];
  case 116:
    return table.data[116];
  case 117:
    return table.data[117];
  case 118:
    return table.data[118];
  case 119:
    return table.data[119];
  case 120:
    return table.data[120];
  case 121:
    return table.data[121];
  case 122:
    return table.data[122];
  case 123:
    return table.data[123];
  case 124:
    return table.data[124];
  case 125:
    return table.data[125];
  case 126:
    return table.data[126];
  case 127:
    return table.data[127];
  case 128:
    return table.data[128];
  case 129:
    return table.data[129];
  case 130:
    return table.data[130];
  case 131:
    return table.data[131];
  case 132:
    return table.data[132];
  case 133:
    return table.data[133];
  case 134:
    return table.data[134];
  case 135:
    return table.data[135];
  case 136:
    return table.data[136];
  case 137:
    return table.data[137];
  case 138:
    return table.data[138];
  case 139:
    return table.data[139];
  case 140:
    return table.data[140];
  case 141:
    return table.data[141];
  case 142:
    return table.data[142];
  case 143:
    return table.data[143];
  case 144:
    return table.data[144];
  case 145:
    return table.data[145];
  case 146:
    return table.data[146];
  case 147:
    return table.data[147];
  case 148:
    return table.data[148];
  case 149:
    return table.data[149];
  case 150:
    return table.data[150];
  case 151:
    return table.data[151];
  case 152:
    return table.data[152];
  case 153:
    return table.data[153];
  case 154:
    return table.data[154];
  case 155:
    return table.data[155];
  case 156:
    return table.data[156];
  case 157:
    return table.data[157];
  case 158:
    return table.data[158];
  case 159:
    return table.data[159];
  case 160:
    return table.data[160];
  case 161:
    return table.data[161];
  case 162:
    return table.data[162];
  case 163:
    return table.data[163];
  case 164:
    return table.data[164];
  case 165:
    return table.data[165];
  case 166:
    return table.data[166];
  case 167:
    return table.data[167];
  case 168:
    return table.data[168];
  case 169:
    return table.data[169];
  case 170:
    return table.data[170];
  case 171:
    return table.data[171];
  case 172:
    return table.data[172];
  case 173:
    return table.data[173];
  case 174:
    return table.data[174];
  case 175:
    return table.data[175];
  case 176:
    return table.data[176];
  case 177:
    return table.data[177];
  case 178:
    return table.data[178];
  case 179:
    return table.data[179];
  case 180:
    return table.data[180];
  case 181:
    return table.data[181];
  case 182:
    return table.data[182];
  case 183:
    return table.data[183];
  case 184:
    return table.data[184];
  case 185:
    return table.data[185];
  case 186:
    return table.data[186];
  case 187:
    return table.data[187];
  case 188:
    return table.data[188];
  case 189:
    return table.data[189];
  case 190:
    return table.data[190];
  case 191:
    return table.data[191];
  case 192:
    return table.data[192];
  case 193:
    return table.data[193];
  case 194:
    return table.data[194];
  case 195:
    return table.data[195];
  case 196:
    return table.data[196];
  case 197:
    return table.data[197];
  case 198:
    return table.data[198];
  case 199:
    return table.data[199];
  case 200:
    return table.data[200];
  case 201:
    return table.data[201];
  case 202:
    return table.data[202];
  case 203:
    return table.data[203];
  case 204:
    return table.data[204];
  case 205:
    return table.data[205];
  case 206:
    return table.data[206];
  case 207:
    return table.data[207];
  case 208:
    return table.data[208];
  case 209:
    return table.data[209];
  case 210:
    return table.data[210];
  case 211:
    return table.data[211];
  case 212:
    return table.data[212];
  case 213:
    return table.data[213];
  case 214:
    return table.data[214];
  case 215:
    return table.data[215];
  case 216:
    return table.data[216];
  case 217:
    return table.data[217];
  case 218:
    return table.data[218];
  case 219:
    return table.data[219];
  case 220:
    return table.data[220];
  case 221:
    return table.data[221];
  case 222:
    return table.data[222];
  case 223:
    return table.data[223];
  case 224:
    return table.data[224];
  case 225:
    return table.data[225];
  case 226:
    return table.data[226];
  case 227:
    return table.data[227];
  case 228:
    return table.data[228];
  case 229:
    return table.data[229];
  case 230:
    return table.data[230];
  case 231:
    return table.data[231];
  case 232:
    return table.data[232];
  case 233:
    return table.data[233];
  case 234:
    return table.data[234];
  case 235:
    return table.data[235];
  case 236:
    return table.data[236];
  case 237:
    return table.data[237];
  case 238:
    return table.data[238];
  case 239:
    return table.data[239];
  case 240:
    return table.data[240];
  case 241:
    return table.data[241];
  case 242:
    return table.data[242];
  case 243:
    return table.data[243];
  case 244:
    return table.data[244];
  case 245:
    return table.data[245];
  case 246:
    return table.data[246];
  case 247:
    return table.data[247];
  case 248:
    return table.data[248];
  case 249:
    return table.data[249];
  case 250:
    return table.data[250];
  case 251:
    return table.data[251];
  case 252:
    return table.data[252];
  case 253:
    return table.data[253];
  case 254:
    return table.data[254];
  case 255:
    return table.data[255];
  default:
    __builtin_unreachable();
  }
}

template <uint32_t S> struct kb_table {
  constexpr kb_table() : data() {
    for (uint32_t b_l = 0; b_l < S / 2; ++b_l) {
      // Need to calculate physical bunch index...
      // ... i.e., position among bunches left-to-right in buffer space
      const uint32_t v =
          std::bit_width(b_l); // Nestedness depth level of physical bunch
      const uint32_t w =
          (S >> v) *
          (v != 0); // Num bunches spaced between bunches in nest level
      const uint32_t o =
          w >> 1; // Offset of nestedness level in physical bunch order
      const uint32_t p =
          b_l - std::bit_floor(b_l);  // Bunch position within nestedness level
      const uint32_t b_p = o + w * p; // Physical bunch index...
      // ... i.e., in left-to-right sequential bunch order

      // Need to calculate buffer position of b_p'th bunch
      const bool epsilon_k_b =
          (b_l != 0); // Correction factor for zeroth bunch...
      // ... i.e., bunch r=s at site k=0
      data[b_l] = (b_p << 1) + std::popcount((S << 1) - b_p) - 1 -
                  epsilon_k_b; // Site index of bunch
    }
  }
  uint16_t data[S / 2];
};
