template <class T>
void TRecSerializator::SetFieldByte(T& RecMem,
    const TFieldSerialDesc& FieldSerialDesc, const uchar& Byte) {
    SetFieldByte(RecMem.GetBf(), RecMem.Len(), FieldSerialDesc, Byte);
}
template <class T>
void TRecSerializator::SetFieldInt(T& RecMem,
    const TFieldSerialDesc& FieldSerialDesc, const int& Int) {
    SetFieldInt(RecMem.GetBf(), RecMem.Len(), FieldSerialDesc, Int);
}
template <class T>
void TRecSerializator::SetFieldInt16(T& RecMem,
    const TFieldSerialDesc& FieldSerialDesc, const int16& Int16) {
    SetFieldInt16(RecMem.GetBf(), RecMem.Len(), FieldSerialDesc, Int16);
}
template <class T>
void TRecSerializator::SetFieldInt64(T& RecMem,
    const TFieldSerialDesc& FieldSerialDesc, const int64& Int64) {
    SetFieldInt64(RecMem.GetBf(), RecMem.Len(), FieldSerialDesc, Int64);
}

template <class T>
void TRecSerializator::SetFieldUInt(T& RecMem,
    const TFieldSerialDesc& FieldSerialDesc, const uint& UInt) {
    SetFieldUInt(RecMem.GetBf(), RecMem.Len(), FieldSerialDesc, UInt);
}
template <class T>
void TRecSerializator::SetFieldUInt16(T& RecMem,
    const TFieldSerialDesc& FieldSerialDesc, const uint16& UInt16) {
    SetFieldUInt16(RecMem.GetBf(), RecMem.Len(), FieldSerialDesc, UInt16);
}
template <class T>
void TRecSerializator::SetFieldUInt64(T& RecMem,
    const TFieldSerialDesc& FieldSerialDesc, const uint64& UInt64) {
    SetFieldUInt64(RecMem.GetBf(), RecMem.Len(), FieldSerialDesc, UInt64);
}

template <class T>
void TRecSerializator::SetFieldStr(T& RecMem,
    const TFieldSerialDesc& FieldSerialDesc, const TStr& Str) {
    SetFieldStr(RecMem.GetBf(), RecMem.Len(), FieldSerialDesc, Str);
}

template <class T>
void TRecSerializator::SetFieldBool(T& RecMem,
    const TFieldSerialDesc& FieldSerialDesc, const bool& Bool) {
    SetFieldBool(RecMem.GetBf(), RecMem.Len(), FieldSerialDesc, Bool);
}

template <class T>
void TRecSerializator::SetFieldFlt(T& RecMem,
    const TFieldSerialDesc& FieldSerialDesc, const double& Flt) {
    SetFieldFlt(RecMem.GetBf(), RecMem.Len(), FieldSerialDesc, Flt);
}
template <class T>
void TRecSerializator::SetFieldSFlt(T& RecMem,
    const TFieldSerialDesc& FieldSerialDesc, const float& Flt) {
    SetFieldSFlt(RecMem.GetBf(), RecMem.Len(), FieldSerialDesc, Flt);
}

template <class T>
void TRecSerializator::SetFieldFltPr(T& RecMem,
    const TFieldSerialDesc& FieldSerialDesc, const TFltPr& FltPr) {
    SetFieldFltPr(RecMem.GetBf(), RecMem.Len(), FieldSerialDesc, FltPr);
}

template <class T>
void TRecSerializator::SetFieldTm(T& RecMem,
    const TFieldSerialDesc& FieldSerialDesc, const TTm& Tm) {
    SetFieldTm(RecMem.GetBf(), RecMem.Len(), FieldSerialDesc, Tm);
}

template <class T>
void TRecSerializator::SetFieldTmMSecs(T& RecMem,
    const TFieldSerialDesc& FieldSerialDesc, const uint64& TmMSecs) {
    SetFieldTmMSecs(RecMem.GetBf(), RecMem.Len(), FieldSerialDesc, TmMSecs);
}

template <class T>
void TRecSerializator::SetFixedJsonVal(T& RecMem,
    const TFieldSerialDesc& FieldSerialDesc, const TFieldDesc& FieldDesc,
    const PJsonVal& JsonVal) {
    SetFixedJsonVal(RecMem.GetBf(), RecMem.Len(), FieldSerialDesc, FieldDesc, JsonVal);
}
