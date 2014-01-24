/**
 * GLib - General C++ Library
 * 
 * Copyright (C) 2014 Jozef Stefan Institut d.o.o.
 *
 * This library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License, version 3,
 * as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 * 
 */

/////////////////////////////////////////////////
// String-Utilities
class TStrUtil {
public:
	static TStr GetStr(const TIntV& IntV, const TStr& DelimiterStr = ",");	
	static TStr GetStr(const TStrIntPrV& StrIntPrV, const TStr& FieldDelimiterStr = ":",
		const TStr& DelimiterStr = ",");	
	static TStr GetStr(const TFltV& FltV, const TStr& DelimiterStr = ",", const TStr& FmtStr = "%g");	
	static TStr GetStr(const TIntFltKdV& IntFltKdV, const TStr& FieldDelimiterStr = ":", 
		const TStr& DelimiterStr = ",", const TStr& FmtStr = "%g");	
	static TStr GetStr(const TStrV& StrV, const TStr& DelimiterStr = ",");	
	static TStr GetStr(const TStrH& StrH, const TStr& FieldDelimiterStr = ":", const TStr& DelimiterStr = ",");	
};
