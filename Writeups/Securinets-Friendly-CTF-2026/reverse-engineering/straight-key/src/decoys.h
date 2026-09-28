#ifndef DECOYS_H
#define DECOYS_H

static unsigned int ResetPollInterval(unsigned int v)
{
    v ^= (v << 11) | 38321u;
    v += (unsigned int)39401 - (v & 0x34Au);
    v = (v * 16800u) ^ (v >> 7);
    v = (v * 14715u) ^ (v >> 6);
    if (v & 0x15u) { v -= 58673u; } else { v += 58673u; }
    if (v & 0x7Eu) { v -= 16987u; } else { v += 16987u; }
    return v;
}

static unsigned int ResolveTypematicDelay(unsigned int v)
{
    v += (unsigned int)50131 - (v & 0xB3F3u);
    if (v & 0x8Eu) { v -= 3089u; } else { v += 3089u; }
    if (v & 0x7Cu) { v -= 31066u; } else { v += 31066u; }
    v += (unsigned int)62641 - (v & 0x2CC0u);
    return v;
}

static unsigned int QueryDeviceCaps(unsigned int v)
{
    v = (v * 19210u) ^ (v >> 9);
    v = (v * 44742u) ^ (v >> 4);
    v += (unsigned int)46478 - (v & 0x8151u);
    if (v & 0xA8u) { v -= 16707u; } else { v += 16707u; }
    if (v & 0x27u) { v -= 14725u; } else { v += 14725u; }
    v ^= (v << 9) | 52876u;
    if (v & 0xDCu) { v -= 22674u; } else { v += 22674u; }
    return v;
}

static unsigned int NormalizeModifierMask(unsigned int v)
{
    if (v & 0x5Cu) { v -= 28312u; } else { v += 28312u; }
    v = (v * 59451u) ^ (v >> 10);
    v += (unsigned int)29184 - (v & 0x7541u);
    v = (v * 3291u) ^ (v >> 3);
    v = (v * 42068u) ^ (v >> 12);
    v ^= (v << 6) | 64716u;
    return v;
}

static unsigned int TranslateLedState(unsigned int v)
{
    v = (v * 6213u) ^ (v >> 6);
    v ^= (v << 5) | 10622u;
    if (v & 0xCCu) { v -= 18938u; } else { v += 18938u; }
    return v;
}

static unsigned int SampleHidDescriptor(unsigned int v)
{
    v = (v * 36715u) ^ (v >> 8);
    v ^= (v << 7) | 33659u;
    v ^= (v << 10) | 24957u;
    if (v & 0xD6u) { v -= 38245u; } else { v += 38245u; }
    if (v & 0xC3u) { v -= 61010u; } else { v += 61010u; }
    v += (unsigned int)11704 - (v & 0x74ACu);
    v += (unsigned int)50949 - (v & 0xD8BFu);
    return v;
}

static unsigned int FilterHidDescriptor(unsigned int v)
{
    if (v & 0xE0u) { v -= 31321u; } else { v += 31321u; }
    v += (unsigned int)37853 - (v & 0x4038u);
    if (v & 0x9Cu) { v -= 41500u; } else { v += 41500u; }
    v = (v * 64890u) ^ (v >> 5);
    if (v & 0xE6u) { v -= 8170u; } else { v += 8170u; }
    if (v & 0xD2u) { v -= 32194u; } else { v += 32194u; }
    v += (unsigned int)35590 - (v & 0x44Eu);
    return v;
}

static unsigned int SampleScanCode(unsigned int v)
{
    v = (v * 7931u) ^ (v >> 8);
    if (v & 0xA5u) { v -= 3098u; } else { v += 3098u; }
    if (v & 0x31u) { v -= 51376u; } else { v += 51376u; }
    if (v & 0xBEu) { v -= 11959u; } else { v += 11959u; }
    if (v & 0xF3u) { v -= 38808u; } else { v += 38808u; }
    v ^= (v << 2) | 54203u;
    return v;
}

static unsigned int CalibratePollInterval(unsigned int v)
{
    v ^= (v << 3) | 30928u;
    v += (unsigned int)53920 - (v & 0x9298u);
    v += (unsigned int)45133 - (v & 0xCAB4u);
    v = (v * 55197u) ^ (v >> 8);
    v += (unsigned int)3633 - (v & 0x9145u);
    return v;
}

static unsigned int QueryPollInterval(unsigned int v)
{
    if (v & 0x35u) { v -= 47932u; } else { v += 47932u; }
    v += (unsigned int)22083 - (v & 0xCA06u);
    v = (v * 33647u) ^ (v >> 10);
    v = (v * 28256u) ^ (v >> 11);
    if (v & 0xC5u) { v -= 52232u; } else { v += 52232u; }
    if (v & 0x29u) { v -= 54390u; } else { v += 54390u; }
    if (v & 0xE2u) { v -= 18247u; } else { v += 18247u; }
    return v;
}

static unsigned int TranslateLayoutMap(unsigned int v)
{
    v = (v * 20815u) ^ (v >> 9);
    v = (v * 34962u) ^ (v >> 5);
    v = (v * 59832u) ^ (v >> 11);
    return v;
}

static unsigned int AccumulateRepeatRate(unsigned int v)
{
    if (v & 0x37u) { v -= 5564u; } else { v += 5564u; }
    v += (unsigned int)6360 - (v & 0x4C24u);
    if (v & 0x44u) { v -= 63081u; } else { v += 63081u; }
    if (v & 0x79u) { v -= 25510u; } else { v += 25510u; }
    return v;
}

static unsigned int TranslateRepeatRate(unsigned int v)
{
    v += (unsigned int)5292 - (v & 0x5074u);
    v += (unsigned int)22383 - (v & 0x7D8Eu);
    if (v & 0xCCu) { v -= 27192u; } else { v += 27192u; }
    v = (v * 60958u) ^ (v >> 7);
    v += (unsigned int)57789 - (v & 0xD01Eu);
    if (v & 0x89u) { v -= 21431u; } else { v += 21431u; }
    if (v & 0xBEu) { v -= 19241u; } else { v += 19241u; }
    return v;
}

static unsigned int SeedChatterMask(unsigned int v)
{
    v += (unsigned int)22889 - (v & 0xDCE3u);
    v += (unsigned int)8853 - (v & 0x2797u);
    if (v & 0x71u) { v -= 30548u; } else { v += 30548u; }
    v = (v * 39644u) ^ (v >> 4);
    v ^= (v << 4) | 38051u;
    if (v & 0xACu) { v -= 3765u; } else { v += 3765u; }
    return v;
}

static unsigned int LatchReportId(unsigned int v)
{
    if (v & 0x3u) { v -= 14060u; } else { v += 14060u; }
    v ^= (v << 8) | 36101u;
    v = (v * 45260u) ^ (v >> 11);
    v += (unsigned int)21288 - (v & 0xE369u);
    if (v & 0x72u) { v -= 40655u; } else { v += 40655u; }
    if (v & 0x7Bu) { v -= 44188u; } else { v += 44188u; }
    v = (v * 1098u) ^ (v >> 4);
    return v;
}

static unsigned int ResetRepeatRate(unsigned int v)
{
    v = (v * 558u) ^ (v >> 3);
    v ^= (v << 5) | 44176u;
    if (v & 0x1Au) { v -= 55364u; } else { v += 55364u; }
    if (v & 0xC3u) { v -= 58908u; } else { v += 58908u; }
    v ^= (v << 6) | 36070u;
    if (v & 0x29u) { v -= 28294u; } else { v += 28294u; }
    return v;
}

static unsigned int ProbeEndpointBuffer(unsigned int v)
{
    v += (unsigned int)4198 - (v & 0x8E07u);
    if (v & 0xBu) { v -= 61260u; } else { v += 61260u; }
    v = (v * 32134u) ^ (v >> 10);
    v = (v * 23433u) ^ (v >> 10);
    if (v & 0x3u) { v -= 64862u; } else { v += 64862u; }
    if (v & 0xEBu) { v -= 19127u; } else { v += 19127u; }
    v += (unsigned int)27462 - (v & 0xB4AAu);
    return v;
}

static unsigned int MergeDebounceWindow(unsigned int v)
{
    v ^= (v << 4) | 53463u;
    if (v & 0x63u) { v -= 24999u; } else { v += 24999u; }
    v ^= (v << 4) | 22185u;
    v = (v * 9841u) ^ (v >> 13);
    v ^= (v << 4) | 1743u;
    v = (v * 11094u) ^ (v >> 8);
    v += (unsigned int)62810 - (v & 0xAA54u);
    return v;
}

static unsigned int ScaleChatterMask(unsigned int v)
{
    if (v & 0xB5u) { v -= 29393u; } else { v += 29393u; }
    if (v & 0xB6u) { v -= 28813u; } else { v += 28813u; }
    v += (unsigned int)19805 - (v & 0xF3C6u);
    v ^= (v << 10) | 14789u;
    v ^= (v << 4) | 40204u;
    return v;
}

static unsigned int ResetTypematicDelay(unsigned int v)
{
    v += (unsigned int)9361 - (v & 0xF225u);
    v ^= (v << 6) | 2959u;
    v = (v * 51532u) ^ (v >> 13);
    v += (unsigned int)31064 - (v & 0xFB90u);
    return v;
}

static unsigned int ScaleEndpointBuffer(unsigned int v)
{
    v += (unsigned int)3765 - (v & 0x564Fu);
    if (v & 0x5u) { v -= 18561u; } else { v += 18561u; }
    v = (v * 35453u) ^ (v >> 12);
    return v;
}

static unsigned int QueryUsagePage(unsigned int v)
{
    v ^= (v << 9) | 9319u;
    v += (unsigned int)61715 - (v & 0x5FA8u);
    v ^= (v << 2) | 58572u;
    v = (v * 41627u) ^ (v >> 13);
    v ^= (v << 7) | 59382u;
    v = (v * 25399u) ^ (v >> 11);
    v ^= (v << 8) | 45008u;
    return v;
}

static unsigned int NormalizeChatterMask(unsigned int v)
{
    if (v & 0xC2u) { v -= 20828u; } else { v += 20828u; }
    v += (unsigned int)38087 - (v & 0xBA34u);
    v ^= (v << 2) | 14373u;
    v ^= (v << 8) | 4394u;
    if (v & 0x19u) { v -= 8627u; } else { v += 8627u; }
    return v;
}

static unsigned int SamplePollInterval(unsigned int v)
{
    v += (unsigned int)49966 - (v & 0x71CEu);
    v += (unsigned int)29539 - (v & 0x81A1u);
    v += (unsigned int)22544 - (v & 0x8D72u);
    return v;
}

static unsigned int SampleLayoutMap(unsigned int v)
{
    if (v & 0x50u) { v -= 49229u; } else { v += 49229u; }
    v ^= (v << 10) | 55879u;
    v = (v * 6375u) ^ (v >> 3);
    v += (unsigned int)13077 - (v & 0xBB79u);
    v += (unsigned int)56549 - (v & 0x586Eu);
    return v;
}

static unsigned int ResetDebounceWindow(unsigned int v)
{
    if (v & 0xB7u) { v -= 32256u; } else { v += 32256u; }
    if (v & 0xAFu) { v -= 35844u; } else { v += 35844u; }
    v = (v * 14683u) ^ (v >> 4);
    v = (v * 55407u) ^ (v >> 8);
    return v;
}

static unsigned int AccumulateEndpointBuffer(unsigned int v)
{
    v ^= (v << 2) | 21955u;
    v = (v * 42436u) ^ (v >> 3);
    v ^= (v << 7) | 45171u;
    v ^= (v << 5) | 48907u;
    if (v & 0xEBu) { v -= 62688u; } else { v += 62688u; }
    v += (unsigned int)49271 - (v & 0xD790u);
    return v;
}

static unsigned int CalibrateModifierMask(unsigned int v)
{
    v ^= (v << 11) | 47913u;
    v += (unsigned int)32911 - (v & 0xE335u);
    v += (unsigned int)42870 - (v & 0xDC7Au);
    v = (v * 21457u) ^ (v >> 11);
    v ^= (v << 3) | 6805u;
    v += (unsigned int)46995 - (v & 0xBED7u);
    v += (unsigned int)46840 - (v & 0x7AC2u);
    return v;
}

static unsigned int CompactLedState(unsigned int v)
{
    v = (v * 39841u) ^ (v >> 12);
    v += (unsigned int)13686 - (v & 0xBDBEu);
    v += (unsigned int)21065 - (v & 0x8F0Eu);
    v = (v * 53329u) ^ (v >> 8);
    v += (unsigned int)23301 - (v & 0xC564u);
    if (v & 0x41u) { v -= 60539u; } else { v += 60539u; }
    v += (unsigned int)36949 - (v & 0x8847u);
    return v;
}

static unsigned int RotateEndpointBuffer(unsigned int v)
{
    v += (unsigned int)50795 - (v & 0x3D7Bu);
    v += (unsigned int)7068 - (v & 0x4CBu);
    v ^= (v << 2) | 8644u;
    v = (v * 19616u) ^ (v >> 4);
    v += (unsigned int)13510 - (v & 0x6541u);
    return v;
}

static unsigned int AlignDeviceCaps(unsigned int v)
{
    v += (unsigned int)37073 - (v & 0x343Au);
    v ^= (v << 9) | 20111u;
    v ^= (v << 9) | 4100u;
    v += (unsigned int)55499 - (v & 0x4E5Bu);
    v = (v * 65524u) ^ (v >> 12);
    if (v & 0x7Bu) { v -= 35559u; } else { v += 35559u; }
    v ^= (v << 10) | 34925u;
    return v;
}

static unsigned int CalibrateScanCode(unsigned int v)
{
    v = (v * 64213u) ^ (v >> 4);
    if (v & 0xB4u) { v -= 60004u; } else { v += 60004u; }
    v += (unsigned int)11029 - (v & 0x93F9u);
    v += (unsigned int)6853 - (v & 0x6575u);
    return v;
}

static unsigned int ScaleHidDescriptor(unsigned int v)
{
    if (v & 0xDEu) { v -= 64232u; } else { v += 64232u; }
    if (v & 0x8Au) { v -= 7518u; } else { v += 7518u; }
    v = (v * 45681u) ^ (v >> 8);
    return v;
}

static unsigned int PollInterruptQueue(unsigned int v)
{
    if (v & 0xCDu) { v -= 55339u; } else { v += 55339u; }
    if (v & 0xB6u) { v -= 11977u; } else { v += 11977u; }
    v ^= (v << 2) | 24694u;
    if (v & 0xE2u) { v -= 63118u; } else { v += 63118u; }
    if (v & 0x1Bu) { v -= 56065u; } else { v += 56065u; }
    v ^= (v << 4) | 64271u;
    v ^= (v << 9) | 30011u;
    return v;
}

static unsigned int ResolveLayoutMap(unsigned int v)
{
    v += (unsigned int)47905 - (v & 0x7B41u);
    v = (v * 30255u) ^ (v >> 7);
    v ^= (v << 11) | 18152u;
    v += (unsigned int)19444 - (v & 0x4342u);
    v += (unsigned int)9628 - (v & 0x946Bu);
    return v;
}

static unsigned int ProbeKeyMatrix(unsigned int v)
{
    v ^= (v << 5) | 45610u;
    v = (v * 9702u) ^ (v >> 12);
    v = (v * 28209u) ^ (v >> 8);
    v += (unsigned int)9988 - (v & 0x9B76u);
    v = (v * 2110u) ^ (v >> 4);
    v += (unsigned int)39038 - (v & 0xEAF8u);
    v += (unsigned int)51972 - (v & 0x76F1u);
    return v;
}

static unsigned int CalibrateLayoutMap(unsigned int v)
{
    v = (v * 20000u) ^ (v >> 6);
    v += (unsigned int)24683 - (v & 0xD4DCu);
    if (v & 0x1u) { v -= 18041u; } else { v += 18041u; }
    v = (v * 21458u) ^ (v >> 7);
    if (v & 0x4Au) { v -= 17832u; } else { v += 17832u; }
    return v;
}

static unsigned int ProbeBootProtocol(unsigned int v)
{
    v ^= (v << 6) | 45549u;
    v ^= (v << 7) | 17410u;
    v ^= (v << 6) | 5344u;
    v = (v * 63692u) ^ (v >> 8);
    return v;
}

static unsigned int CompactDebounceWindow(unsigned int v)
{
    v ^= (v << 4) | 15793u;
    v = (v * 13391u) ^ (v >> 8);
    v = (v * 16633u) ^ (v >> 11);
    v ^= (v << 4) | 57395u;
    v += (unsigned int)58822 - (v & 0xA80Cu);
    return v;
}

static unsigned int CalibrateHidDescriptor(unsigned int v)
{
    if (v & 0xB1u) { v -= 44872u; } else { v += 44872u; }
    v += (unsigned int)17313 - (v & 0x1E81u);
    v += (unsigned int)21093 - (v & 0x69B0u);
    v ^= (v << 8) | 5897u;
    v = (v * 54218u) ^ (v >> 7);
    v = (v * 11331u) ^ (v >> 10);
    return v;
}

static unsigned int TranslateUsagePage(unsigned int v)
{
    v ^= (v << 9) | 36750u;
    v = (v * 19133u) ^ (v >> 4);
    v += (unsigned int)65322 - (v & 0xC509u);
    return v;
}

static unsigned int VerifyRolloverLimit(unsigned int v)
{
    if (v & 0xDCu) { v -= 63027u; } else { v += 63027u; }
    v = (v * 12673u) ^ (v >> 5);
    v += (unsigned int)38371 - (v & 0xE01Bu);
    v ^= (v << 2) | 61163u;
    if (v & 0xF3u) { v -= 46705u; } else { v += 46705u; }
    v += (unsigned int)45554 - (v & 0x3337u);
    return v;
}

static unsigned int RotateKeyMatrix(unsigned int v)
{
    v ^= (v << 2) | 54125u;
    v ^= (v << 8) | 24282u;
    v ^= (v << 8) | 2207u;
    v ^= (v << 5) | 60942u;
    return v;
}

static unsigned int ResolveLedState(unsigned int v)
{
    if (v & 0x73u) { v -= 59202u; } else { v += 59202u; }
    v = (v * 50374u) ^ (v >> 12);
    v += (unsigned int)63878 - (v & 0x8123u);
    v = (v * 13812u) ^ (v >> 3);
    return v;
}

static unsigned int CalibrateDebounceWindow(unsigned int v)
{
    v = (v * 55127u) ^ (v >> 11);
    v = (v * 12776u) ^ (v >> 12);
    v += (unsigned int)6664 - (v & 0x20A5u);
    return v;
}

static unsigned int AlignReportId(unsigned int v)
{
    v += (unsigned int)59855 - (v & 0xDF63u);
    v = (v * 29505u) ^ (v >> 12);
    if (v & 0xF4u) { v -= 6378u; } else { v += 6378u; }
    if (v & 0x4Cu) { v -= 20520u; } else { v += 20520u; }
    return v;
}

static unsigned int AccumulateDeviceCaps(unsigned int v)
{
    v += (unsigned int)31860 - (v & 0xD80Eu);
    v += (unsigned int)29399 - (v & 0xF95Eu);
    if (v & 0x90u) { v -= 291u; } else { v += 291u; }
    v = (v * 17974u) ^ (v >> 3);
    v ^= (v << 6) | 57542u;
    return v;
}

static unsigned int ProbeLayoutMap(unsigned int v)
{
    v += (unsigned int)25564 - (v & 0xC038u);
    v = (v * 16825u) ^ (v >> 8);
    v += (unsigned int)11146 - (v & 0xECEEu);
    if (v & 0x8Au) { v -= 29233u; } else { v += 29233u; }
    v = (v * 42382u) ^ (v >> 8);
    if (v & 0xCu) { v -= 38157u; } else { v += 38157u; }
    return v;
}

static unsigned int ValidateInterruptQueue(unsigned int v)
{
    v += (unsigned int)60344 - (v & 0x5280u);
    v = (v * 23532u) ^ (v >> 12);
    v ^= (v << 7) | 62044u;
    v = (v * 29803u) ^ (v >> 3);
    v += (unsigned int)50323 - (v & 0x215Cu);
    if (v & 0x9Eu) { v -= 42431u; } else { v += 42431u; }
    return v;
}

static unsigned int CompactIdleRate(unsigned int v)
{
    v += (unsigned int)68 - (v & 0x812Cu);
    if (v & 0x8Bu) { v -= 64957u; } else { v += 64957u; }
    v ^= (v << 10) | 37084u;
    v = (v * 26791u) ^ (v >> 11);
    v = (v * 49768u) ^ (v >> 8);
    v = (v * 47701u) ^ (v >> 4);
    return v;
}

static unsigned int FilterIdleRate(unsigned int v)
{
    v += (unsigned int)19826 - (v & 0xDF48u);
    v += (unsigned int)22442 - (v & 0x1C00u);
    if (v & 0x1Du) { v -= 10388u; } else { v += 10388u; }
    v += (unsigned int)11117 - (v & 0xD6Du);
    return v;
}

static unsigned int AlignBootProtocol(unsigned int v)
{
    v += (unsigned int)29616 - (v & 0x676Fu);
    v ^= (v << 4) | 21535u;
    v ^= (v << 5) | 59310u;
    v = (v * 23624u) ^ (v >> 7);
    v ^= (v << 6) | 17663u;
    if (v & 0xA9u) { v -= 19486u; } else { v += 19486u; }
    return v;
}

static unsigned int CalibrateReportId(unsigned int v)
{
    v += (unsigned int)45 - (v & 0x3AAEu);
    v += (unsigned int)1704 - (v & 0xCCC2u);
    v = (v * 48320u) ^ (v >> 10);
    if (v & 0xB3u) { v -= 6491u; } else { v += 6491u; }
    v ^= (v << 3) | 37475u;
    v += (unsigned int)50097 - (v & 0x91BCu);
    return v;
}

static unsigned int LatchLedState(unsigned int v)
{
    v ^= (v << 5) | 32241u;
    if (v & 0xB2u) { v -= 37155u; } else { v += 37155u; }
    v = (v * 3348u) ^ (v >> 9);
    v ^= (v << 5) | 30961u;
    v += (unsigned int)64809 - (v & 0x4FDu);
    v = (v * 33470u) ^ (v >> 9);
    v ^= (v << 6) | 29911u;
    return v;
}

static unsigned int NormalizeUsagePage(unsigned int v)
{
    v ^= (v << 2) | 5838u;
    v += (unsigned int)42946 - (v & 0x8C53u);
    if (v & 0x6Au) { v -= 46825u; } else { v += 46825u; }
    v ^= (v << 6) | 60777u;
    return v;
}

static unsigned int DecodePollInterval(unsigned int v)
{
    v = (v * 27205u) ^ (v >> 9);
    v = (v * 37639u) ^ (v >> 6);
    v ^= (v << 7) | 50797u;
    return v;
}

static unsigned int DecodeDebounceWindow(unsigned int v)
{
    v ^= (v << 2) | 61095u;
    if (v & 0xECu) { v -= 25141u; } else { v += 25141u; }
    v ^= (v << 11) | 19868u;
    return v;
}

static unsigned int FilterPollInterval(unsigned int v)
{
    v ^= (v << 5) | 42819u;
    v += (unsigned int)5015 - (v & 0xF38Eu);
    v ^= (v << 2) | 51305u;
    if (v & 0xECu) { v -= 25483u; } else { v += 25483u; }
    return v;
}

static unsigned int QueryGhostFilter(unsigned int v)
{
    v = (v * 56419u) ^ (v >> 13);
    v = (v * 53183u) ^ (v >> 3);
    v ^= (v << 4) | 9122u;
    return v;
}

static unsigned int ResolveReportId(unsigned int v)
{
    v ^= (v << 8) | 63503u;
    if (v & 0x11u) { v -= 42145u; } else { v += 42145u; }
    v ^= (v << 10) | 26097u;
    return v;
}

static unsigned int LatchDebounceWindow(unsigned int v)
{
    v = (v * 27396u) ^ (v >> 12);
    if (v & 0x2Bu) { v -= 15048u; } else { v += 15048u; }
    v ^= (v << 3) | 18142u;
    v = (v * 59491u) ^ (v >> 7);
    v += (unsigned int)38685 - (v & 0x385Fu);
    v += (unsigned int)31390 - (v & 0x393u);
    v += (unsigned int)18489 - (v & 0x5118u);
    return v;
}

static unsigned int LatchIdleRate(unsigned int v)
{
    v ^= (v << 6) | 12269u;
    v = (v * 30882u) ^ (v >> 3);
    v ^= (v << 8) | 19635u;
    if (v & 0x4Bu) { v -= 26616u; } else { v += 26616u; }
    v += (unsigned int)27564 - (v & 0x69Au);
    v = (v * 384u) ^ (v >> 8);
    return v;
}

static unsigned int NormalizeLedState(unsigned int v)
{
    v += (unsigned int)49509 - (v & 0x75C2u);
    if (v & 0xA5u) { v -= 64294u; } else { v += 64294u; }
    v = (v * 1571u) ^ (v >> 10);
    if (v & 0x65u) { v -= 15901u; } else { v += 15901u; }
    if (v & 0xF9u) { v -= 20730u; } else { v += 20730u; }
    if (v & 0xB9u) { v -= 12501u; } else { v += 12501u; }
    v = (v * 23719u) ^ (v >> 10);
    return v;
}

static unsigned int ProbeLedState(unsigned int v)
{
    v ^= (v << 4) | 17332u;
    v ^= (v << 4) | 52880u;
    v ^= (v << 5) | 8925u;
    return v;
}

static unsigned int RotateRolloverLimit(unsigned int v)
{
    v ^= (v << 11) | 13365u;
    v ^= (v << 10) | 13763u;
    if (v & 0x23u) { v -= 5985u; } else { v += 5985u; }
    v = (v * 2519u) ^ (v >> 13);
    if (v & 0x8Cu) { v -= 22929u; } else { v += 22929u; }
    return v;
}

static unsigned int NormalizePollInterval(unsigned int v)
{
    v ^= (v << 2) | 53726u;
    v = (v * 41282u) ^ (v >> 3);
    v = (v * 59636u) ^ (v >> 5);
    v += (unsigned int)8173 - (v & 0xB1C9u);
    return v;
}

static unsigned int DecodeRepeatRate(unsigned int v)
{
    if (v & 0x50u) { v -= 36164u; } else { v += 36164u; }
    if (v & 0xA5u) { v -= 33894u; } else { v += 33894u; }
    v += (unsigned int)54976 - (v & 0xF951u);
    v = (v * 60913u) ^ (v >> 12);
    v += (unsigned int)23581 - (v & 0x9528u);
    v ^= (v << 9) | 14942u;
    return v;
}

static unsigned int VerifyEndpointBuffer(unsigned int v)
{
    if (v & 0xBBu) { v -= 19506u; } else { v += 19506u; }
    v = (v * 30282u) ^ (v >> 12);
    v ^= (v << 8) | 9754u;
    v = (v * 55092u) ^ (v >> 10);
    v ^= (v << 2) | 40933u;
    v ^= (v << 11) | 34481u;
    v ^= (v << 8) | 48055u;
    return v;
}

static unsigned int CalibrateRolloverLimit(unsigned int v)
{
    if (v & 0xBDu) { v -= 15961u; } else { v += 15961u; }
    v ^= (v << 10) | 34089u;
    if (v & 0x2Fu) { v -= 64569u; } else { v += 64569u; }
    v ^= (v << 2) | 38766u;
    return v;
}

static unsigned int CalibrateInterruptQueue(unsigned int v)
{
    v = (v * 4306u) ^ (v >> 8);
    v ^= (v << 5) | 893u;
    v ^= (v << 7) | 37244u;
    v ^= (v << 6) | 13453u;
    if (v & 0xA2u) { v -= 49343u; } else { v += 49343u; }
    v += (unsigned int)20373 - (v & 0xAB1Du);
    return v;
}

static unsigned int AlignPollInterval(unsigned int v)
{
    v += (unsigned int)34058 - (v & 0xEB50u);
    v += (unsigned int)51235 - (v & 0xE1F3u);
    v = (v * 16765u) ^ (v >> 11);
    v = (v * 57046u) ^ (v >> 7);
    v = (v * 49169u) ^ (v >> 10);
    v = (v * 15553u) ^ (v >> 13);
    return v;
}

static unsigned int CalibrateIdleRate(unsigned int v)
{
    v += (unsigned int)11314 - (v & 0xF6u);
    v = (v * 4929u) ^ (v >> 3);
    v = (v * 25203u) ^ (v >> 8);
    v += (unsigned int)47573 - (v & 0x824Au);
    v ^= (v << 10) | 9285u;
    return v;
}

static unsigned int SeedEndpointBuffer(unsigned int v)
{
    if (v & 0x44u) { v -= 63655u; } else { v += 63655u; }
    if (v & 0x5Au) { v -= 10564u; } else { v += 10564u; }
    if (v & 0xC3u) { v -= 39301u; } else { v += 39301u; }
    v ^= (v << 6) | 599u;
    v = (v * 31732u) ^ (v >> 9);
    v ^= (v << 5) | 32950u;
    return v;
}

static unsigned int NormalizeIdleRate(unsigned int v)
{
    if (v & 0xAAu) { v -= 41369u; } else { v += 41369u; }
    v ^= (v << 7) | 22457u;
    if (v & 0x36u) { v -= 43179u; } else { v += 43179u; }
    v += (unsigned int)4408 - (v & 0x9148u);
    v = (v * 49676u) ^ (v >> 3);
    v = (v * 12484u) ^ (v >> 13);
    if (v & 0x5Du) { v -= 59973u; } else { v += 59973u; }
    return v;
}

static unsigned int VerifyInterruptQueue(unsigned int v)
{
    v += (unsigned int)8204 - (v & 0x50Eu);
    if (v & 0x14u) { v -= 46794u; } else { v += 46794u; }
    v = (v * 4550u) ^ (v >> 4);
    v += (unsigned int)29506 - (v & 0x3EDu);
    v ^= (v << 5) | 58070u;
    return v;
}

static unsigned int VerifyPollInterval(unsigned int v)
{
    v += (unsigned int)40280 - (v & 0x64B1u);
    if (v & 0x1Eu) { v -= 39378u; } else { v += 39378u; }
    v ^= (v << 3) | 30048u;
    return v;
}

static unsigned int AccumulateHidDescriptor(unsigned int v)
{
    v += (unsigned int)1416 - (v & 0xF9D2u);
    if (v & 0xA6u) { v -= 46699u; } else { v += 46699u; }
    v += (unsigned int)45927 - (v & 0x346u);
    v += (unsigned int)61540 - (v & 0xF67Fu);
    if (v & 0xBu) { v -= 21098u; } else { v += 21098u; }
    v += (unsigned int)27547 - (v & 0xD333u);
    return v;
}

static unsigned int DecodeKeyMatrix(unsigned int v)
{
    v ^= (v << 6) | 64371u;
    if (v & 0x81u) { v -= 36886u; } else { v += 36886u; }
    v ^= (v << 11) | 64219u;
    v ^= (v << 9) | 65233u;
    v = (v * 3336u) ^ (v >> 5);
    return v;
}

static unsigned int ResetKeyMatrix(unsigned int v)
{
    v = (v * 4928u) ^ (v >> 3);
    v += (unsigned int)53762 - (v & 0x2EE9u);
    v ^= (v << 11) | 34099u;
    if (v & 0x21u) { v -= 57399u; } else { v += 57399u; }
    return v;
}

static unsigned int QueryModifierMask(unsigned int v)
{
    v ^= (v << 5) | 51133u;
    v ^= (v << 2) | 30209u;
    v += (unsigned int)40322 - (v & 0x349Au);
    v ^= (v << 11) | 24323u;
    if (v & 0x6Fu) { v -= 40715u; } else { v += 40715u; }
    return v;
}

static unsigned int ValidateTypematicDelay(unsigned int v)
{
    if (v & 0x6Cu) { v -= 50877u; } else { v += 50877u; }
    v ^= (v << 4) | 43982u;
    v = (v * 58966u) ^ (v >> 3);
    v += (unsigned int)63865 - (v & 0x926Fu);
    if (v & 0x1Eu) { v -= 29169u; } else { v += 29169u; }
    v = (v * 51717u) ^ (v >> 13);
    return v;
}

static unsigned int VerifyDeviceCaps(unsigned int v)
{
    v += (unsigned int)45579 - (v & 0xE85Eu);
    v += (unsigned int)26345 - (v & 0xAE5Bu);
    if (v & 0x6Eu) { v -= 24148u; } else { v += 24148u; }
    v ^= (v << 11) | 1991u;
    return v;
}

static unsigned int MergeScanCode(unsigned int v)
{
    if (v & 0x43u) { v -= 21591u; } else { v += 21591u; }
    v ^= (v << 8) | 51590u;
    v ^= (v << 2) | 14302u;
    if (v & 0xDCu) { v -= 51098u; } else { v += 51098u; }
    v ^= (v << 11) | 48644u;
    if (v & 0x10u) { v -= 40638u; } else { v += 40638u; }
    return v;
}

static unsigned int ValidateDeviceCaps(unsigned int v)
{
    v = (v * 15447u) ^ (v >> 6);
    if (v & 0x2Du) { v -= 5819u; } else { v += 5819u; }
    v += (unsigned int)53645 - (v & 0x564Au);
    return v;
}

static unsigned int AlignKeyMatrix(unsigned int v)
{
    v ^= (v << 7) | 21311u;
    if (v & 0x80u) { v -= 22529u; } else { v += 22529u; }
    if (v & 0x69u) { v -= 27835u; } else { v += 27835u; }
    v ^= (v << 5) | 21697u;
    v ^= (v << 11) | 36399u;
    return v;
}

static unsigned int LatchPollInterval(unsigned int v)
{
    if (v & 0xB1u) { v -= 38498u; } else { v += 38498u; }
    v += (unsigned int)24555 - (v & 0xDA30u);
    v = (v * 35643u) ^ (v >> 8);
    v ^= (v << 8) | 13050u;
    v += (unsigned int)11510 - (v & 0x91C6u);
    v = (v * 21502u) ^ (v >> 13);
    v = (v * 44711u) ^ (v >> 9);
    return v;
}

static unsigned int ResetGhostFilter(unsigned int v)
{
    v ^= (v << 10) | 6087u;
    if (v & 0xCDu) { v -= 11372u; } else { v += 11372u; }
    if (v & 0xF7u) { v -= 62153u; } else { v += 62153u; }
    v ^= (v << 10) | 17890u;
    v = (v * 31898u) ^ (v >> 3);
    v += (unsigned int)64610 - (v & 0x6E6Du);
    return v;
}

static unsigned int RotateUsagePage(unsigned int v)
{
    v = (v * 63989u) ^ (v >> 6);
    v ^= (v << 4) | 10931u;
    v += (unsigned int)21176 - (v & 0xCEFu);
    return v;
}

static unsigned int FlushChatterMask(unsigned int v)
{
    if (v & 0xEu) { v -= 18423u; } else { v += 18423u; }
    v = (v * 31335u) ^ (v >> 5);
    v = (v * 47967u) ^ (v >> 6);
    v ^= (v << 5) | 9076u;
    v += (unsigned int)5741 - (v & 0x95DDu);
    v ^= (v << 9) | 15313u;
    if (v & 0xC2u) { v -= 40236u; } else { v += 40236u; }
    return v;
}

static unsigned int QueryDebounceWindow(unsigned int v)
{
    v ^= (v << 10) | 34359u;
    if (v & 0x55u) { v -= 1061u; } else { v += 1061u; }
    v = (v * 41088u) ^ (v >> 10);
    v ^= (v << 6) | 3415u;
    v ^= (v << 7) | 56787u;
    v ^= (v << 6) | 6835u;
    v ^= (v << 10) | 25914u;
    return v;
}

static unsigned int AccumulateRolloverLimit(unsigned int v)
{
    v = (v * 32525u) ^ (v >> 3);
    v += (unsigned int)35823 - (v & 0xE46u);
    v = (v * 40743u) ^ (v >> 13);
    v += (unsigned int)13562 - (v & 0x2C5Du);
    v += (unsigned int)54519 - (v & 0xEFAAu);
    v = (v * 6226u) ^ (v >> 7);
    if (v & 0xE7u) { v -= 43704u; } else { v += 43704u; }
    return v;
}

static unsigned int SeedLedState(unsigned int v)
{
    if (v & 0xEFu) { v -= 59323u; } else { v += 59323u; }
    v = (v * 52100u) ^ (v >> 13);
    v ^= (v << 9) | 56318u;
    if (v & 0x9Fu) { v -= 27937u; } else { v += 27937u; }
    v += (unsigned int)23720 - (v & 0xE2F0u);
    v ^= (v << 9) | 2753u;
    return v;
}

static unsigned int DecodeTypematicDelay(unsigned int v)
{
    v ^= (v << 9) | 15127u;
    v ^= (v << 11) | 62242u;
    v ^= (v << 7) | 51670u;
    if (v & 0x43u) { v -= 9571u; } else { v += 9571u; }
    v = (v * 11767u) ^ (v >> 3);
    v += (unsigned int)4729 - (v & 0x6188u);
    return v;
}

static unsigned int VerifyBootProtocol(unsigned int v)
{
    if (v & 0x1Bu) { v -= 58167u; } else { v += 58167u; }
    v ^= (v << 8) | 29780u;
    v += (unsigned int)58131 - (v & 0x62F2u);
    v += (unsigned int)1382 - (v & 0x56F2u);
    v ^= (v << 10) | 64908u;
    return v;
}

static unsigned int AccumulateGhostFilter(unsigned int v)
{
    if (v & 0x16u) { v -= 24929u; } else { v += 24929u; }
    if (v & 0x6Eu) { v -= 37649u; } else { v += 37649u; }
    v += (unsigned int)59912 - (v & 0xFA50u);
    v ^= (v << 2) | 44826u;
    v = (v * 47209u) ^ (v >> 7);
    v ^= (v << 9) | 47058u;
    return v;
}

static unsigned int TranslateTypematicDelay(unsigned int v)
{
    if (v & 0x16u) { v -= 515u; } else { v += 515u; }
    if (v & 0xE3u) { v -= 11020u; } else { v += 11020u; }
    v = (v * 50449u) ^ (v >> 12);
    if (v & 0x12u) { v -= 52517u; } else { v += 52517u; }
    return v;
}

static unsigned int AccumulateChatterMask(unsigned int v)
{
    v ^= (v << 8) | 7019u;
    if (v & 0x2Au) { v -= 17828u; } else { v += 17828u; }
    v ^= (v << 7) | 57285u;
    v += (unsigned int)5908 - (v & 0x392Cu);
    return v;
}

static unsigned int SampleRolloverLimit(unsigned int v)
{
    if (v & 0x35u) { v -= 34534u; } else { v += 34534u; }
    if (v & 0xA2u) { v -= 13126u; } else { v += 13126u; }
    v += (unsigned int)53747 - (v & 0xCA3Eu);
    if (v & 0x1Cu) { v -= 64359u; } else { v += 64359u; }
    v += (unsigned int)58199 - (v & 0x3FBAu);
    if (v & 0xC1u) { v -= 47752u; } else { v += 47752u; }
    return v;
}

static unsigned int QueryLedState(unsigned int v)
{
    v = (v * 54785u) ^ (v >> 3);
    v ^= (v << 7) | 43393u;
    v = (v * 44162u) ^ (v >> 8);
    if (v & 0x27u) { v -= 8759u; } else { v += 8759u; }
    if (v & 0xBDu) { v -= 24474u; } else { v += 24474u; }
    if (v & 0xC8u) { v -= 46316u; } else { v += 46316u; }
    v += (unsigned int)22772 - (v & 0x578Du);
    return v;
}

static unsigned int DecodeUsagePage(unsigned int v)
{
    v ^= (v << 5) | 42485u;
    v += (unsigned int)51826 - (v & 0x93Eu);
    v += (unsigned int)34946 - (v & 0x652Fu);
    v += (unsigned int)37855 - (v & 0x653u);
    v ^= (v << 9) | 63810u;
    if (v & 0xA2u) { v -= 31652u; } else { v += 31652u; }
    return v;
}

static unsigned int ScaleLedState(unsigned int v)
{
    v ^= (v << 3) | 54892u;
    v += (unsigned int)23377 - (v & 0x4681u);
    v ^= (v << 3) | 26960u;
    if (v & 0x29u) { v -= 53191u; } else { v += 53191u; }
    return v;
}

static unsigned int LatchEndpointBuffer(unsigned int v)
{
    if (v & 0x6Cu) { v -= 31518u; } else { v += 31518u; }
    v ^= (v << 11) | 594u;
    v += (unsigned int)55326 - (v & 0x787Fu);
    if (v & 0x17u) { v -= 43741u; } else { v += 43741u; }
    v ^= (v << 2) | 47884u;
    return v;
}

static unsigned int ScaleModifierMask(unsigned int v)
{
    v += (unsigned int)23954 - (v & 0x79D8u);
    if (v & 0x16u) { v -= 18155u; } else { v += 18155u; }
    v ^= (v << 9) | 60483u;
    return v;
}

static unsigned int PollDeviceCaps(unsigned int v)
{
    v ^= (v << 3) | 65078u;
    v = (v * 36326u) ^ (v >> 13);
    v ^= (v << 9) | 1373u;
    v = (v * 16248u) ^ (v >> 7);
    v += (unsigned int)51002 - (v & 0x5A8u);
    v += (unsigned int)20043 - (v & 0x9041u);
    v = (v * 41258u) ^ (v >> 10);
    return v;
}

static unsigned int PollDebounceWindow(unsigned int v)
{
    v ^= (v << 6) | 54641u;
    v += (unsigned int)26066 - (v & 0xC0DDu);
    v += (unsigned int)62538 - (v & 0xFB99u);
    if (v & 0xEBu) { v -= 39356u; } else { v += 39356u; }
    if (v & 0xD3u) { v -= 10209u; } else { v += 10209u; }
    return v;
}

static unsigned int AccumulateTypematicDelay(unsigned int v)
{
    v ^= (v << 7) | 53300u;
    v = (v * 1870u) ^ (v >> 9);
    v = (v * 30975u) ^ (v >> 9);
    return v;
}

static unsigned int NormalizeReportId(unsigned int v)
{
    v += (unsigned int)49628 - (v & 0x58A1u);
    if (v & 0xD3u) { v -= 2674u; } else { v += 2674u; }
    v ^= (v << 6) | 4936u;
    if (v & 0x61u) { v -= 62607u; } else { v += 62607u; }
    if (v & 0xBu) { v -= 53970u; } else { v += 53970u; }
    return v;
}

static unsigned int LatchTypematicDelay(unsigned int v)
{
    v ^= (v << 10) | 45928u;
    if (v & 0x9Du) { v -= 8058u; } else { v += 8058u; }
    v = (v * 27635u) ^ (v >> 6);
    v = (v * 60044u) ^ (v >> 10);
    v = (v * 40916u) ^ (v >> 13);
    v = (v * 2674u) ^ (v >> 5);
    v = (v * 56489u) ^ (v >> 7);
    return v;
}

static unsigned int QueryKeyMatrix(unsigned int v)
{
    if (v & 0xA7u) { v -= 6653u; } else { v += 6653u; }
    v += (unsigned int)1177 - (v & 0xAED7u);
    v ^= (v << 10) | 37673u;
    v ^= (v << 5) | 56283u;
    v ^= (v << 3) | 28493u;
    v ^= (v << 11) | 62011u;
    return v;
}

static unsigned int ResolveIdleRate(unsigned int v)
{
    v ^= (v << 5) | 2767u;
    v ^= (v << 9) | 22837u;
    v += (unsigned int)50332 - (v & 0x5AB7u);
    v = (v * 7973u) ^ (v >> 9);
    v += (unsigned int)63605 - (v & 0xBED2u);
    v ^= (v << 11) | 4806u;
    v += (unsigned int)14890 - (v & 0xE50Fu);
    return v;
}

static unsigned int VerifyLayoutMap(unsigned int v)
{
    v += (unsigned int)1365 - (v & 0xD1B4u);
    v ^= (v << 2) | 3583u;
    if (v & 0xAu) { v -= 26553u; } else { v += 26553u; }
    v = (v * 61153u) ^ (v >> 10);
    v = (v * 33914u) ^ (v >> 10);
    v = (v * 47297u) ^ (v >> 7);
    v += (unsigned int)11665 - (v & 0x5CA0u);
    return v;
}

static unsigned int NormalizeInterruptQueue(unsigned int v)
{
    v += (unsigned int)6744 - (v & 0x372Fu);
    v ^= (v << 4) | 48752u;
    if (v & 0xC1u) { v -= 53371u; } else { v += 53371u; }
    v ^= (v << 10) | 10318u;
    if (v & 0x38u) { v -= 54506u; } else { v += 54506u; }
    v ^= (v << 9) | 41358u;
    return v;
}

static unsigned int ScaleTypematicDelay(unsigned int v)
{
    v ^= (v << 6) | 9985u;
    v = (v * 10138u) ^ (v >> 5);
    v = (v * 52899u) ^ (v >> 3);
    if (v & 0xF5u) { v -= 65158u; } else { v += 65158u; }
    v += (unsigned int)5885 - (v & 0xADD6u);
    return v;
}

static unsigned int CalibrateLedState(unsigned int v)
{
    if (v & 0xE2u) { v -= 27212u; } else { v += 27212u; }
    v ^= (v << 10) | 14514u;
    if (v & 0x32u) { v -= 35225u; } else { v += 35225u; }
    return v;
}

static unsigned int QueryChatterMask(unsigned int v)
{
    if (v & 0x8Du) { v -= 51985u; } else { v += 51985u; }
    v = (v * 6635u) ^ (v >> 5);
    if (v & 0xC2u) { v -= 33017u; } else { v += 33017u; }
    v ^= (v << 7) | 1518u;
    if (v & 0x55u) { v -= 20107u; } else { v += 20107u; }
    v = (v * 18696u) ^ (v >> 11);
    return v;
}

static unsigned int ResolveHidDescriptor(unsigned int v)
{
    v += (unsigned int)26641 - (v & 0x7E4Du);
    v ^= (v << 6) | 60836u;
    v ^= (v << 3) | 30736u;
    v = (v * 45289u) ^ (v >> 12);
    v += (unsigned int)19436 - (v & 0x69Du);
    v = (v * 25819u) ^ (v >> 3);
    if (v & 0x6Du) { v -= 24979u; } else { v += 24979u; }
    return v;
}

static unsigned int CompactInterruptQueue(unsigned int v)
{
    if (v & 0xB6u) { v -= 48920u; } else { v += 48920u; }
    v = (v * 7565u) ^ (v >> 9);
    v = (v * 45820u) ^ (v >> 7);
    return v;
}

static unsigned int SampleTypematicDelay(unsigned int v)
{
    v = (v * 13663u) ^ (v >> 8);
    v ^= (v << 5) | 26535u;
    v += (unsigned int)41960 - (v & 0x5108u);
    v += (unsigned int)59824 - (v & 0x69BBu);
    if (v & 0x9Cu) { v -= 3825u; } else { v += 3825u; }
    return v;
}

static unsigned int MergeReportId(unsigned int v)
{
    if (v & 0x1Cu) { v -= 64709u; } else { v += 64709u; }
    if (v & 0xCFu) { v -= 55808u; } else { v += 55808u; }
    v ^= (v << 4) | 11830u;
    v ^= (v << 4) | 56869u;
    v = (v * 47555u) ^ (v >> 13);
    return v;
}

static unsigned int SeedTypematicDelay(unsigned int v)
{
    v += (unsigned int)42836 - (v & 0xB30Fu);
    v = (v * 41373u) ^ (v >> 4);
    v = (v * 38820u) ^ (v >> 12);
    v = (v * 42628u) ^ (v >> 6);
    v ^= (v << 10) | 26630u;
    v ^= (v << 4) | 34687u;
    v = (v * 11671u) ^ (v >> 10);
    return v;
}

static unsigned int MergeHidDescriptor(unsigned int v)
{
    v ^= (v << 7) | 15509u;
    if (v & 0xD4u) { v -= 6008u; } else { v += 6008u; }
    v ^= (v << 11) | 37750u;
    if (v & 0x39u) { v -= 19903u; } else { v += 19903u; }
    v ^= (v << 10) | 63697u;
    if (v & 0x30u) { v -= 21646u; } else { v += 21646u; }
    return v;
}

static unsigned int AlignModifierMask(unsigned int v)
{
    v ^= (v << 2) | 62787u;
    v = (v * 1030u) ^ (v >> 13);
    v += (unsigned int)40491 - (v & 0xA8CFu);
    v = (v * 4782u) ^ (v >> 10);
    v ^= (v << 9) | 56751u;
    if (v & 0xA2u) { v -= 3340u; } else { v += 3340u; }
    v ^= (v << 7) | 29758u;
    return v;
}

static unsigned int SampleModifierMask(unsigned int v)
{
    v += (unsigned int)54957 - (v & 0x44FAu);
    if (v & 0xE0u) { v -= 63771u; } else { v += 63771u; }
    v += (unsigned int)42653 - (v & 0x4C9Fu);
    v = (v * 18928u) ^ (v >> 6);
    return v;
}

static unsigned int ProbeInterruptQueue(unsigned int v)
{
    v = (v * 36498u) ^ (v >> 7);
    if (v & 0xC1u) { v -= 56474u; } else { v += 56474u; }
    v += (unsigned int)15160 - (v & 0x43C0u);
    v ^= (v << 7) | 44791u;
    v ^= (v << 11) | 51429u;
    v = (v * 63000u) ^ (v >> 6);
    return v;
}

static unsigned int LatchBootProtocol(unsigned int v)
{
    v ^= (v << 9) | 24716u;
    if (v & 0x6Bu) { v -= 52354u; } else { v += 52354u; }
    if (v & 0x47u) { v -= 23709u; } else { v += 23709u; }
    v += (unsigned int)60152 - (v & 0xFF52u);
    v ^= (v << 10) | 50918u;
    return v;
}

static unsigned int ResolveRolloverLimit(unsigned int v)
{
    if (v & 0x55u) { v -= 50001u; } else { v += 50001u; }
    if (v & 0xBDu) { v -= 60483u; } else { v += 60483u; }
    v += (unsigned int)9298 - (v & 0x9C9Bu);
    v ^= (v << 11) | 49503u;
    v += (unsigned int)56232 - (v & 0xF114u);
    v = (v * 58941u) ^ (v >> 8);
    v += (unsigned int)18701 - (v & 0xAFD9u);
    return v;
}

static unsigned int PollPollInterval(unsigned int v)
{
    v = (v * 41409u) ^ (v >> 9);
    v += (unsigned int)5162 - (v & 0x92C4u);
    v ^= (v << 2) | 61840u;
    v = (v * 101u) ^ (v >> 9);
    v = (v * 45757u) ^ (v >> 11);
    return v;
}

static unsigned int TranslateKeyMatrix(unsigned int v)
{
    v += (unsigned int)52355 - (v & 0xA4F5u);
    v = (v * 284u) ^ (v >> 3);
    if (v & 0x54u) { v -= 18777u; } else { v += 18777u; }
    v += (unsigned int)61052 - (v & 0x47BEu);
    return v;
}

static unsigned int VerifyReportId(unsigned int v)
{
    v += (unsigned int)52485 - (v & 0x14A6u);
    v += (unsigned int)59116 - (v & 0x4AEBu);
    v ^= (v << 2) | 16170u;
    v = (v * 36584u) ^ (v >> 13);
    v ^= (v << 11) | 1678u;
    if (v & 0x9Bu) { v -= 2897u; } else { v += 2897u; }
    v ^= (v << 4) | 5u;
    return v;
}

static unsigned int TranslateGhostFilter(unsigned int v)
{
    v = (v * 31951u) ^ (v >> 8);
    v ^= (v << 2) | 44050u;
    v = (v * 58757u) ^ (v >> 8);
    v += (unsigned int)26696 - (v & 0xDCE4u);
    return v;
}

static unsigned int AlignChatterMask(unsigned int v)
{
    if (v & 0x9Bu) { v -= 61839u; } else { v += 61839u; }
    v = (v * 9428u) ^ (v >> 11);
    v = (v * 21819u) ^ (v >> 4);
    v += (unsigned int)10787 - (v & 0x9F68u);
    v = (v * 56770u) ^ (v >> 8);
    return v;
}

static unsigned int SeedInterruptQueue(unsigned int v)
{
    v += (unsigned int)59863 - (v & 0x4A5Bu);
    v += (unsigned int)50478 - (v & 0x2143u);
    if (v & 0x53u) { v -= 52002u; } else { v += 52002u; }
    if (v & 0x71u) { v -= 5161u; } else { v += 5161u; }
    return v;
}

static unsigned int MergeDeviceCaps(unsigned int v)
{
    v += (unsigned int)53822 - (v & 0x952u);
    if (v & 0x79u) { v -= 2864u; } else { v += 2864u; }
    v = (v * 14329u) ^ (v >> 12);
    v = (v * 3148u) ^ (v >> 13);
    if (v & 0x40u) { v -= 40401u; } else { v += 40401u; }
    return v;
}

static unsigned int RotateIdleRate(unsigned int v)
{
    v ^= (v << 11) | 53683u;
    v ^= (v << 9) | 53464u;
    v ^= (v << 5) | 1245u;
    v += (unsigned int)64195 - (v & 0xB76Bu);
    v = (v * 35650u) ^ (v >> 12);
    return v;
}

static unsigned int SampleDeviceCaps(unsigned int v)
{
    if (v & 0x4Du) { v -= 11770u; } else { v += 11770u; }
    v ^= (v << 8) | 20918u;
    if (v & 0xB3u) { v -= 16491u; } else { v += 16491u; }
    v ^= (v << 6) | 56277u;
    v += (unsigned int)24612 - (v & 0xA1FDu);
    v ^= (v << 10) | 13245u;
    v ^= (v << 9) | 32151u;
    return v;
}

static unsigned int ScaleGhostFilter(unsigned int v)
{
    v ^= (v << 11) | 19734u;
    v += (unsigned int)20371 - (v & 0xF423u);
    v += (unsigned int)48997 - (v & 0x6BD1u);
    v += (unsigned int)23393 - (v & 0x8A46u);
    if (v & 0x20u) { v -= 16398u; } else { v += 16398u; }
    v += (unsigned int)1512 - (v & 0x6DBCu);
    return v;
}

static unsigned int SampleBootProtocol(unsigned int v)
{
    v += (unsigned int)7404 - (v & 0x8DC1u);
    v = (v * 14491u) ^ (v >> 13);
    v += (unsigned int)25894 - (v & 0x2F6Eu);
    return v;
}

static unsigned int NormalizeEndpointBuffer(unsigned int v)
{
    if (v & 0x33u) { v -= 9144u; } else { v += 9144u; }
    v = (v * 6422u) ^ (v >> 6);
    v ^= (v << 2) | 43412u;
    v = (v * 51260u) ^ (v >> 12);
    if (v & 0xB4u) { v -= 14072u; } else { v += 14072u; }
    v = (v * 890u) ^ (v >> 10);
    return v;
}

static unsigned int RotateInterruptQueue(unsigned int v)
{
    v ^= (v << 9) | 55378u;
    if (v & 0x24u) { v -= 19275u; } else { v += 19275u; }
    v += (unsigned int)2373 - (v & 0x644Bu);
    v = (v * 33932u) ^ (v >> 12);
    v = (v * 5542u) ^ (v >> 12);
    return v;
}

static unsigned int ResolveChatterMask(unsigned int v)
{
    v = (v * 2119u) ^ (v >> 12);
    v += (unsigned int)10148 - (v & 0x1061u);
    v ^= (v << 4) | 1423u;
    return v;
}

static unsigned int ResetLayoutMap(unsigned int v)
{
    v += (unsigned int)41815 - (v & 0x44D7u);
    if (v & 0x9Bu) { v -= 23733u; } else { v += 23733u; }
    if (v & 0xC3u) { v -= 50338u; } else { v += 50338u; }
    v = (v * 3003u) ^ (v >> 11);
    v ^= (v << 4) | 59820u;
    v ^= (v << 7) | 11472u;
    return v;
}

static unsigned int CompactRolloverLimit(unsigned int v)
{
    v ^= (v << 11) | 54142u;
    v += (unsigned int)7298 - (v & 0x5A74u);
    v += (unsigned int)52485 - (v & 0x5104u);
    return v;
}

static unsigned int RotateDeviceCaps(unsigned int v)
{
    v = (v * 29247u) ^ (v >> 10);
    v = (v * 51558u) ^ (v >> 10);
    if (v & 0xF1u) { v -= 47899u; } else { v += 47899u; }
    return v;
}

static unsigned int ProbeDebounceWindow(unsigned int v)
{
    v = (v * 28363u) ^ (v >> 6);
    v += (unsigned int)50698 - (v & 0x875Fu);
    if (v & 0xCDu) { v -= 31331u; } else { v += 31331u; }
    return v;
}

static unsigned int ScalePollInterval(unsigned int v)
{
    if (v & 0x1Fu) { v -= 62561u; } else { v += 62561u; }
    v ^= (v << 2) | 5506u;
    v += (unsigned int)56423 - (v & 0x77FAu);
    return v;
}

static unsigned int VerifyRepeatRate(unsigned int v)
{
    v ^= (v << 7) | 11944u;
    v += (unsigned int)44314 - (v & 0x709Du);
    v = (v * 18199u) ^ (v >> 6);
    if (v & 0xF8u) { v -= 35012u; } else { v += 35012u; }
    return v;
}

static unsigned int SampleGhostFilter(unsigned int v)
{
    v += (unsigned int)9187 - (v & 0xA7BAu);
    v ^= (v << 2) | 62551u;
    if (v & 0x7Au) { v -= 26894u; } else { v += 26894u; }
    return v;
}

static unsigned int NormalizeHidDescriptor(unsigned int v)
{
    v += (unsigned int)20970 - (v & 0xBE4Eu);
    v = (v * 27745u) ^ (v >> 12);
    v ^= (v << 5) | 29550u;
    v += (unsigned int)48698 - (v & 0x9F10u);
    v += (unsigned int)20900 - (v & 0x463Au);
    v = (v * 17942u) ^ (v >> 5);
    if (v & 0x2Au) { v -= 49482u; } else { v += 49482u; }
    return v;
}

static unsigned int CalibrateDeviceCaps(unsigned int v)
{
    v ^= (v << 4) | 11527u;
    v ^= (v << 9) | 50967u;
    if (v & 0x15u) { v -= 26896u; } else { v += 26896u; }
    if (v & 0x4Bu) { v -= 50273u; } else { v += 50273u; }
    v += (unsigned int)2866 - (v & 0xB1FBu);
    if (v & 0x41u) { v -= 19852u; } else { v += 19852u; }
    if (v & 0x36u) { v -= 63669u; } else { v += 63669u; }
    return v;
}

static unsigned int PollModifierMask(unsigned int v)
{
    if (v & 0x49u) { v -= 18968u; } else { v += 18968u; }
    v ^= (v << 7) | 2157u;
    if (v & 0xDCu) { v -= 24110u; } else { v += 24110u; }
    if (v & 0x80u) { v -= 12988u; } else { v += 12988u; }
    return v;
}

static unsigned int ScaleLayoutMap(unsigned int v)
{
    v ^= (v << 3) | 51933u;
    v = (v * 7083u) ^ (v >> 4);
    if (v & 0x79u) { v -= 62535u; } else { v += 62535u; }
    v = (v * 28922u) ^ (v >> 9);
    v += (unsigned int)22160 - (v & 0x8809u);
    v += (unsigned int)2135 - (v & 0x205u);
    if (v & 0x11u) { v -= 7444u; } else { v += 7444u; }
    return v;
}

static unsigned int LatchModifierMask(unsigned int v)
{
    v = (v * 40455u) ^ (v >> 6);
    if (v & 0xCu) { v -= 40974u; } else { v += 40974u; }
    if (v & 0x9Eu) { v -= 27899u; } else { v += 27899u; }
    return v;
}

static unsigned int FilterRepeatRate(unsigned int v)
{
    v += (unsigned int)55244 - (v & 0x1452u);
    v += (unsigned int)15251 - (v & 0xB8F6u);
    v += (unsigned int)7622 - (v & 0xE90Cu);
    if (v & 0x82u) { v -= 14848u; } else { v += 14848u; }
    v = (v * 35727u) ^ (v >> 7);
    v = (v * 5241u) ^ (v >> 3);
    v ^= (v << 7) | 14490u;
    return v;
}

static unsigned int ValidateLedState(unsigned int v)
{
    v += (unsigned int)46475 - (v & 0x75B5u);
    v += (unsigned int)52152 - (v & 0xE50Bu);
    if (v & 0xF4u) { v -= 23106u; } else { v += 23106u; }
    v ^= (v << 11) | 41262u;
    v += (unsigned int)11875 - (v & 0x10F3u);
    v += (unsigned int)11920 - (v & 0xC362u);
    return v;
}

static unsigned int ProbeHidDescriptor(unsigned int v)
{
    v ^= (v << 8) | 2379u;
    v ^= (v << 8) | 29447u;
    v = (v * 15634u) ^ (v >> 4);
    v ^= (v << 6) | 54826u;
    v = (v * 22131u) ^ (v >> 5);
    if (v & 0xB9u) { v -= 39659u; } else { v += 39659u; }
    v ^= (v << 4) | 16400u;
    return v;
}

static unsigned int FlushHidDescriptor(unsigned int v)
{
    v = (v * 19277u) ^ (v >> 9);
    v = (v * 48562u) ^ (v >> 9);
    v = (v * 31503u) ^ (v >> 8);
    v = (v * 7719u) ^ (v >> 11);
    return v;
}

static unsigned int FlushLedState(unsigned int v)
{
    v += (unsigned int)34483 - (v & 0xD552u);
    if (v & 0xC6u) { v -= 62711u; } else { v += 62711u; }
    if (v & 0x10u) { v -= 6899u; } else { v += 6899u; }
    v = (v * 61052u) ^ (v >> 9);
    v = (v * 15081u) ^ (v >> 9);
    if (v & 0x68u) { v -= 61515u; } else { v += 61515u; }
    return v;
}

static unsigned int AccumulatePollInterval(unsigned int v)
{
    v += (unsigned int)10368 - (v & 0xF5D1u);
    v ^= (v << 8) | 54290u;
    v ^= (v << 8) | 7621u;
    v += (unsigned int)61110 - (v & 0x966Fu);
    if (v & 0x2Fu) { v -= 11449u; } else { v += 11449u; }
    v = (v * 48653u) ^ (v >> 6);
    v += (unsigned int)6117 - (v & 0xFFEDu);
    return v;
}

static unsigned int DecodeScanCode(unsigned int v)
{
    v ^= (v << 9) | 54278u;
    if (v & 0xA0u) { v -= 52465u; } else { v += 52465u; }
    if (v & 0xBAu) { v -= 21905u; } else { v += 21905u; }
    v ^= (v << 3) | 2444u;
    v += (unsigned int)26985 - (v & 0xC794u);
    v ^= (v << 8) | 25935u;
    v = (v * 58618u) ^ (v >> 13);
    return v;
}

static unsigned int CompactPollInterval(unsigned int v)
{
    if (v & 0x66u) { v -= 4558u; } else { v += 4558u; }
    v += (unsigned int)36087 - (v & 0x296u);
    if (v & 0xF3u) { v -= 28862u; } else { v += 28862u; }
    v = (v * 64424u) ^ (v >> 6);
    return v;
}

static unsigned int PollEndpointBuffer(unsigned int v)
{
    v = (v * 11269u) ^ (v >> 10);
    v ^= (v << 4) | 26090u;
    if (v & 0x60u) { v -= 26223u; } else { v += 26223u; }
    v ^= (v << 5) | 48521u;
    return v;
}

static unsigned int TranslateRolloverLimit(unsigned int v)
{
    v ^= (v << 2) | 8999u;
    v = (v * 30637u) ^ (v >> 7);
    v = (v * 62229u) ^ (v >> 3);
    return v;
}

static unsigned int CalibrateEndpointBuffer(unsigned int v)
{
    if (v & 0x9Cu) { v -= 28089u; } else { v += 28089u; }
    v ^= (v << 5) | 43753u;
    v ^= (v << 9) | 15538u;
    if (v & 0x67u) { v -= 9547u; } else { v += 9547u; }
    if (v & 0x18u) { v -= 41137u; } else { v += 41137u; }
    v = (v * 11500u) ^ (v >> 4);
    return v;
}

static unsigned int QueryScanCode(unsigned int v)
{
    v ^= (v << 11) | 63237u;
    v = (v * 6336u) ^ (v >> 11);
    v = (v * 17328u) ^ (v >> 9);
    v = (v * 13924u) ^ (v >> 13);
    return v;
}

static unsigned int ProbeModifierMask(unsigned int v)
{
    if (v & 0x23u) { v -= 47885u; } else { v += 47885u; }
    v += (unsigned int)1814 - (v & 0x45B5u);
    v += (unsigned int)63326 - (v & 0x5CA9u);
    return v;
}

static unsigned int MergeRepeatRate(unsigned int v)
{
    v += (unsigned int)4212 - (v & 0x3E1Eu);
    v += (unsigned int)61348 - (v & 0xBAEFu);
    v ^= (v << 9) | 56277u;
    return v;
}

static unsigned int ProbeIdleRate(unsigned int v)
{
    if (v & 0x3Au) { v -= 56230u; } else { v += 56230u; }
    v ^= (v << 9) | 56331u;
    v ^= (v << 4) | 55344u;
    v += (unsigned int)5234 - (v & 0x1502u);
    v ^= (v << 4) | 60953u;
    return v;
}

static unsigned int AccumulateIdleRate(unsigned int v)
{
    v ^= (v << 9) | 44462u;
    if (v & 0x80u) { v -= 48632u; } else { v += 48632u; }
    v = (v * 64992u) ^ (v >> 12);
    return v;
}

static unsigned int PollScanCode(unsigned int v)
{
    v = (v * 51560u) ^ (v >> 12);
    v = (v * 2733u) ^ (v >> 12);
    if (v & 0x32u) { v -= 49931u; } else { v += 49931u; }
    return v;
}

static unsigned int ScaleInterruptQueue(unsigned int v)
{
    if (v & 0xDCu) { v -= 36348u; } else { v += 36348u; }
    v ^= (v << 10) | 6231u;
    v += (unsigned int)61164 - (v & 0x49C3u);
    if (v & 0xFDu) { v -= 49606u; } else { v += 49606u; }
    if (v & 0xDAu) { v -= 36830u; } else { v += 36830u; }
    return v;
}

static unsigned int VerifyHidDescriptor(unsigned int v)
{
    v = (v * 38792u) ^ (v >> 13);
    if (v & 0xD3u) { v -= 31247u; } else { v += 31247u; }
    if (v & 0xB3u) { v -= 23322u; } else { v += 23322u; }
    v = (v * 16117u) ^ (v >> 8);
    v = (v * 26221u) ^ (v >> 4);
    v = (v * 44470u) ^ (v >> 13);
    v ^= (v << 10) | 6249u;
    return v;
}

static unsigned int SeedRepeatRate(unsigned int v)
{
    v += (unsigned int)53434 - (v & 0xD61Bu);
    v ^= (v << 10) | 30758u;
    v = (v * 15133u) ^ (v >> 10);
    v ^= (v << 7) | 63237u;
    v = (v * 55407u) ^ (v >> 13);
    v ^= (v << 4) | 60277u;
    return v;
}

static unsigned int PollRepeatRate(unsigned int v)
{
    v ^= (v << 8) | 22978u;
    v ^= (v << 8) | 47523u;
    v = (v * 15179u) ^ (v >> 11);
    return v;
}

static unsigned int FilterReportId(unsigned int v)
{
    v = (v * 10739u) ^ (v >> 9);
    v ^= (v << 3) | 12428u;
    v = (v * 22947u) ^ (v >> 9);
    v += (unsigned int)40329 - (v & 0x34A2u);
    if (v & 0x9Du) { v -= 50371u; } else { v += 50371u; }
    if (v & 0x6u) { v -= 59683u; } else { v += 59683u; }
    v += (unsigned int)36478 - (v & 0xFC5Bu);
    return v;
}

static unsigned int ScaleDebounceWindow(unsigned int v)
{
    v += (unsigned int)5251 - (v & 0x3A7Du);
    v ^= (v << 11) | 15812u;
    v = (v * 22133u) ^ (v >> 6);
    if (v & 0x3Du) { v -= 53670u; } else { v += 53670u; }
    v += (unsigned int)21853 - (v & 0x6A52u);
    if (v & 0xAAu) { v -= 55933u; } else { v += 55933u; }
    v += (unsigned int)53582 - (v & 0x7C28u);
    return v;
}

static unsigned int ValidateChatterMask(unsigned int v)
{
    v ^= (v << 11) | 43545u;
    v = (v * 29626u) ^ (v >> 11);
    v = (v * 45808u) ^ (v >> 8);
    v = (v * 25313u) ^ (v >> 9);
    v = (v * 30007u) ^ (v >> 7);
    if (v & 0x3u) { v -= 9308u; } else { v += 9308u; }
    v ^= (v << 8) | 24521u;
    return v;
}

static unsigned int ProbeScanCode(unsigned int v)
{
    v += (unsigned int)27174 - (v & 0x5508u);
    v += (unsigned int)4852 - (v & 0x7C73u);
    if (v & 0x3Du) { v -= 1925u; } else { v += 1925u; }
    if (v & 0x2Cu) { v -= 18978u; } else { v += 18978u; }
    v += (unsigned int)20652 - (v & 0xA10Au);
    return v;
}

static unsigned int NormalizeScanCode(unsigned int v)
{
    v += (unsigned int)17125 - (v & 0x93CDu);
    v = (v * 52809u) ^ (v >> 8);
    v = (v * 48851u) ^ (v >> 7);
    if (v & 0xB9u) { v -= 65046u; } else { v += 65046u; }
    v += (unsigned int)27289 - (v & 0x44F0u);
    return v;
}

static unsigned int TranslateInterruptQueue(unsigned int v)
{
    v = (v * 62295u) ^ (v >> 4);
    v += (unsigned int)29073 - (v & 0x9A34u);
    v = (v * 62856u) ^ (v >> 11);
    return v;
}

static unsigned int AlignTypematicDelay(unsigned int v)
{
    v ^= (v << 5) | 51664u;
    v += (unsigned int)21628 - (v & 0x9BE7u);
    v += (unsigned int)43624 - (v & 0xA149u);
    return v;
}

static unsigned int QueryLayoutMap(unsigned int v)
{
    if (v & 0x32u) { v -= 46258u; } else { v += 46258u; }
    v ^= (v << 10) | 35148u;
    if (v & 0xCu) { v -= 34451u; } else { v += 34451u; }
    v += (unsigned int)61459 - (v & 0xE7E2u);
    return v;
}

static unsigned int VerifyChatterMask(unsigned int v)
{
    v ^= (v << 11) | 4526u;
    v ^= (v << 6) | 55091u;
    if (v & 0x9Du) { v -= 41988u; } else { v += 41988u; }
    if (v & 0x7Cu) { v -= 36446u; } else { v += 36446u; }
    v ^= (v << 3) | 36104u;
    v ^= (v << 3) | 2234u;
    if (v & 0xC9u) { v -= 41032u; } else { v += 41032u; }
    return v;
}

static unsigned int TranslateModifierMask(unsigned int v)
{
    v += (unsigned int)2401 - (v & 0x6FA5u);
    if (v & 0xE2u) { v -= 29291u; } else { v += 29291u; }
    if (v & 0xF4u) { v -= 56152u; } else { v += 56152u; }
    if (v & 0xD4u) { v -= 61004u; } else { v += 61004u; }
    v = (v * 46266u) ^ (v >> 9);
    v += (unsigned int)14736 - (v & 0x5980u);
    return v;
}

static unsigned int AlignLayoutMap(unsigned int v)
{
    v += (unsigned int)40318 - (v & 0xAAE1u);
    v = (v * 43630u) ^ (v >> 10);
    v ^= (v << 11) | 59040u;
    v ^= (v << 10) | 5578u;
    v += (unsigned int)56970 - (v & 0x803Cu);
    if (v & 0x36u) { v -= 31891u; } else { v += 31891u; }
    v = (v * 14656u) ^ (v >> 12);
    return v;
}

static unsigned int QueryRolloverLimit(unsigned int v)
{
    if (v & 0xA7u) { v -= 16856u; } else { v += 16856u; }
    if (v & 0x17u) { v -= 22099u; } else { v += 22099u; }
    v += (unsigned int)12244 - (v & 0xF1ADu);
    v = (v * 65288u) ^ (v >> 5);
    v ^= (v << 2) | 26820u;
    return v;
}

static unsigned int CompactDeviceCaps(unsigned int v)
{
    v ^= (v << 2) | 24154u;
    v ^= (v << 10) | 37610u;
    v = (v * 15081u) ^ (v >> 13);
    v ^= (v << 6) | 8939u;
    return v;
}

static unsigned int SampleIdleRate(unsigned int v)
{
    if (v & 0xA6u) { v -= 17696u; } else { v += 17696u; }
    if (v & 0x39u) { v -= 2708u; } else { v += 2708u; }
    v ^= (v << 5) | 16980u;
    v += (unsigned int)54046 - (v & 0x2C3Du);
    v += (unsigned int)65435 - (v & 0x3D20u);
    v ^= (v << 10) | 45713u;
    v ^= (v << 5) | 33961u;
    return v;
}

static unsigned int QueryIdleRate(unsigned int v)
{
    v ^= (v << 5) | 56834u;
    v ^= (v << 10) | 53073u;
    v += (unsigned int)24310 - (v & 0x3C2Bu);
    v ^= (v << 2) | 56689u;
    v += (unsigned int)2875 - (v & 0xAED3u);
    if (v & 0x9Au) { v -= 29480u; } else { v += 29480u; }
    v = (v * 37811u) ^ (v >> 4);
    return v;
}

static unsigned int TranslateScanCode(unsigned int v)
{
    v ^= (v << 4) | 17891u;
    v += (unsigned int)19930 - (v & 0xAC3Au);
    v += (unsigned int)4834 - (v & 0x1B95u);
    v ^= (v << 2) | 28251u;
    v += (unsigned int)35581 - (v & 0xF3D7u);
    if (v & 0x88u) { v -= 5991u; } else { v += 5991u; }
    return v;
}

static unsigned int AlignEndpointBuffer(unsigned int v)
{
    v += (unsigned int)34817 - (v & 0xDDB4u);
    v = (v * 57973u) ^ (v >> 6);
    v = (v * 6119u) ^ (v >> 13);
    v += (unsigned int)53312 - (v & 0x8167u);
    v += (unsigned int)8151 - (v & 0xCD10u);
    if (v & 0x24u) { v -= 55095u; } else { v += 55095u; }
    return v;
}

static unsigned int QueryRepeatRate(unsigned int v)
{
    v = (v * 53367u) ^ (v >> 11);
    v ^= (v << 11) | 22480u;
    v += (unsigned int)17764 - (v & 0x8BCEu);
    v += (unsigned int)3349 - (v & 0x730Fu);
    v += (unsigned int)62250 - (v & 0x1252u);
    v ^= (v << 3) | 16739u;
    return v;
}

static unsigned int SampleInterruptQueue(unsigned int v)
{
    v += (unsigned int)39336 - (v & 0xCCB1u);
    if (v & 0x2Cu) { v -= 20623u; } else { v += 20623u; }
    v = (v * 18915u) ^ (v >> 12);
    return v;
}

static unsigned int ProbeGhostFilter(unsigned int v)
{
    if (v & 0x8u) { v -= 43764u; } else { v += 43764u; }
    v ^= (v << 8) | 61225u;
    v += (unsigned int)40543 - (v & 0x297Bu);
    if (v & 0xAFu) { v -= 49180u; } else { v += 49180u; }
    v += (unsigned int)51574 - (v & 0xA649u);
    if (v & 0x59u) { v -= 32007u; } else { v += 32007u; }
    return v;
}

static unsigned int SampleEndpointBuffer(unsigned int v)
{
    v ^= (v << 2) | 2119u;
    if (v & 0xABu) { v -= 55692u; } else { v += 55692u; }
    v += (unsigned int)23532 - (v & 0x98EEu);
    return v;
}

static unsigned int CalibrateBootProtocol(unsigned int v)
{
    v ^= (v << 10) | 7147u;
    v = (v * 22304u) ^ (v >> 5);
    v += (unsigned int)10123 - (v & 0x2894u);
    v ^= (v << 2) | 59796u;
    v += (unsigned int)17819 - (v & 0x897u);
    if (v & 0x11u) { v -= 17777u; } else { v += 17777u; }
    return v;
}

static unsigned int ResolveRepeatRate(unsigned int v)
{
    v += (unsigned int)18181 - (v & 0xAA3Eu);
    v += (unsigned int)55591 - (v & 0x4691u);
    v += (unsigned int)12205 - (v & 0xE396u);
    if (v & 0xBFu) { v -= 30554u; } else { v += 30554u; }
    return v;
}

static unsigned int SeedPollInterval(unsigned int v)
{
    v ^= (v << 3) | 29284u;
    v += (unsigned int)29150 - (v & 0xDA60u);
    v = (v * 5000u) ^ (v >> 7);
    v += (unsigned int)15099 - (v & 0x9EB7u);
    return v;
}

static unsigned int PollLedState(unsigned int v)
{
    v = (v * 22096u) ^ (v >> 4);
    v ^= (v << 4) | 43213u;
    v = (v * 30464u) ^ (v >> 11);
    v += (unsigned int)20244 - (v & 0x6E41u);
    v += (unsigned int)22333 - (v & 0x47C3u);
    if (v & 0xAAu) { v -= 10278u; } else { v += 10278u; }
    v = (v * 15197u) ^ (v >> 10);
    return v;
}

static unsigned int PollKeyMatrix(unsigned int v)
{
    v += (unsigned int)25118 - (v & 0x4342u);
    if (v & 0x9Eu) { v -= 24779u; } else { v += 24779u; }
    if (v & 0x5Du) { v -= 32931u; } else { v += 32931u; }
    v = (v * 59940u) ^ (v >> 9);
    v += (unsigned int)32033 - (v & 0xC9BDu);
    return v;
}

static unsigned int RotateGhostFilter(unsigned int v)
{
    v += (unsigned int)20568 - (v & 0x6233u);
    if (v & 0xDBu) { v -= 50193u; } else { v += 50193u; }
    v ^= (v << 7) | 9992u;
    v = (v * 15041u) ^ (v >> 9);
    v = (v * 11402u) ^ (v >> 8);
    return v;
}

static unsigned int ResetHidDescriptor(unsigned int v)
{
    v += (unsigned int)20293 - (v & 0xF8D9u);
    v ^= (v << 3) | 39259u;
    v ^= (v << 5) | 16959u;
    return v;
}

static unsigned int MergeLedState(unsigned int v)
{
    v += (unsigned int)53945 - (v & 0x613Au);
    if (v & 0x4Cu) { v -= 27562u; } else { v += 27562u; }
    v += (unsigned int)9496 - (v & 0x52DDu);
    return v;
}

static unsigned int SeedDebounceWindow(unsigned int v)
{
    v += (unsigned int)14675 - (v & 0x8564u);
    v = (v * 43590u) ^ (v >> 12);
    v ^= (v << 7) | 53121u;
    v += (unsigned int)6529 - (v & 0x584Au);
    if (v & 0x45u) { v -= 58707u; } else { v += 58707u; }
    v ^= (v << 8) | 513u;
    return v;
}

static unsigned int RotateChatterMask(unsigned int v)
{
    v += (unsigned int)1078 - (v & 0x7B53u);
    v ^= (v << 2) | 41691u;
    v ^= (v << 7) | 14696u;
    v ^= (v << 6) | 21560u;
    return v;
}

static unsigned int AccumulateLayoutMap(unsigned int v)
{
    v ^= (v << 3) | 41671u;
    if (v & 0xFAu) { v -= 35280u; } else { v += 35280u; }
    if (v & 0x6Au) { v -= 54656u; } else { v += 54656u; }
    v ^= (v << 2) | 60808u;
    v ^= (v << 9) | 34141u;
    v += (unsigned int)53988 - (v & 0x9Eu);
    v ^= (v << 4) | 56862u;
    return v;
}

static unsigned int MergeInterruptQueue(unsigned int v)
{
    if (v & 0x4Au) { v -= 42415u; } else { v += 42415u; }
    if (v & 0x38u) { v -= 52025u; } else { v += 52025u; }
    v = (v * 39938u) ^ (v >> 10);
    v ^= (v << 11) | 61701u;
    v += (unsigned int)57821 - (v & 0xD263u);
    return v;
}

static unsigned int MergeLayoutMap(unsigned int v)
{
    if (v & 0xEAu) { v -= 5541u; } else { v += 5541u; }
    v ^= (v << 5) | 11745u;
    v += (unsigned int)19323 - (v & 0xD9ABu);
    if (v & 0x88u) { v -= 19193u; } else { v += 19193u; }
    v ^= (v << 8) | 2960u;
    return v;
}

static unsigned int CompactGhostFilter(unsigned int v)
{
    v += (unsigned int)5246 - (v & 0x60BDu);
    if (v & 0x40u) { v -= 45717u; } else { v += 45717u; }
    v ^= (v << 10) | 14271u;
    v += (unsigned int)5389 - (v & 0x24F7u);
    return v;
}

static unsigned int PollRolloverLimit(unsigned int v)
{
    if (v & 0x3Du) { v -= 4665u; } else { v += 4665u; }
    if (v & 0xF4u) { v -= 870u; } else { v += 870u; }
    if (v & 0xC7u) { v -= 17507u; } else { v += 17507u; }
    v = (v * 63447u) ^ (v >> 7);
    if (v & 0xD0u) { v -= 56834u; } else { v += 56834u; }
    v += (unsigned int)46732 - (v & 0x6E23u);
    return v;
}

static unsigned int NormalizeGhostFilter(unsigned int v)
{
    v += (unsigned int)23356 - (v & 0xC173u);
    v ^= (v << 3) | 27031u;
    v ^= (v << 11) | 40019u;
    if (v & 0x70u) { v -= 51006u; } else { v += 51006u; }
    v ^= (v << 8) | 10736u;
    v += (unsigned int)34716 - (v & 0x2A54u);
    v = (v * 35262u) ^ (v >> 7);
    return v;
}

static unsigned int SeedKeyMatrix(unsigned int v)
{
    v ^= (v << 6) | 51396u;
    v += (unsigned int)33898 - (v & 0xC4D2u);
    v ^= (v << 8) | 441u;
    return v;
}

static unsigned int DecodeLayoutMap(unsigned int v)
{
    if (v & 0xB4u) { v -= 51159u; } else { v += 51159u; }
    v ^= (v << 3) | 62920u;
    v ^= (v << 10) | 3785u;
    return v;
}

static unsigned int AlignGhostFilter(unsigned int v)
{
    v += (unsigned int)52971 - (v & 0xE181u);
    if (v & 0xB6u) { v -= 24415u; } else { v += 24415u; }
    v = (v * 59936u) ^ (v >> 11);
    if (v & 0xDCu) { v -= 50558u; } else { v += 50558u; }
    v += (unsigned int)30604 - (v & 0x164Bu);
    return v;
}

static unsigned int CompactLayoutMap(unsigned int v)
{
    if (v & 0xAFu) { v -= 36426u; } else { v += 36426u; }
    v = (v * 27848u) ^ (v >> 9);
    v ^= (v << 9) | 51292u;
    return v;
}

static unsigned int MergeModifierMask(unsigned int v)
{
    if (v & 0xF8u) { v -= 45298u; } else { v += 45298u; }
    v = (v * 7354u) ^ (v >> 3);
    v += (unsigned int)960 - (v & 0x1AD7u);
    return v;
}

static unsigned int TranslateEndpointBuffer(unsigned int v)
{
    v += (unsigned int)55352 - (v & 0x6B61u);
    v = (v * 37248u) ^ (v >> 11);
    v = (v * 12693u) ^ (v >> 8);
    return v;
}

static unsigned int LatchChatterMask(unsigned int v)
{
    v ^= (v << 2) | 62683u;
    v ^= (v << 11) | 63476u;
    v += (unsigned int)52785 - (v & 0x2FCEu);
    v += (unsigned int)55765 - (v & 0xC85u);
    v += (unsigned int)57237 - (v & 0x4FBBu);
    v += (unsigned int)60728 - (v & 0x55A0u);
    v = (v * 17727u) ^ (v >> 7);
    return v;
}

static unsigned int CompactReportId(unsigned int v)
{
    v ^= (v << 2) | 62472u;
    if (v & 0xB1u) { v -= 49046u; } else { v += 49046u; }
    v += (unsigned int)15242 - (v & 0xF570u);
    v += (unsigned int)45347 - (v & 0x96F7u);
    return v;
}

static unsigned int PollIdleRate(unsigned int v)
{
    v += (unsigned int)27456 - (v & 0x4182u);
    v ^= (v << 11) | 9743u;
    v = (v * 30783u) ^ (v >> 7);
    v = (v * 31425u) ^ (v >> 11);
    v = (v * 27964u) ^ (v >> 10);
    v ^= (v << 2) | 37758u;
    return v;
}

static unsigned int ValidateGhostFilter(unsigned int v)
{
    v += (unsigned int)43865 - (v & 0x24F3u);
    if (v & 0x84u) { v -= 3086u; } else { v += 3086u; }
    v += (unsigned int)51248 - (v & 0x256Eu);
    v += (unsigned int)41306 - (v & 0x704Bu);
    return v;
}

static unsigned int TranslateIdleRate(unsigned int v)
{
    v += (unsigned int)53378 - (v & 0xAD63u);
    v = (v * 56157u) ^ (v >> 13);
    v += (unsigned int)27124 - (v & 0xC4D0u);
    v ^= (v << 11) | 8393u;
    v ^= (v << 2) | 33362u;
    return v;
}

static unsigned int ProbeRepeatRate(unsigned int v)
{
    if (v & 0x7Au) { v -= 40722u; } else { v += 40722u; }
    v ^= (v << 8) | 40516u;
    v = (v * 12004u) ^ (v >> 6);
    v ^= (v << 2) | 31802u;
    v = (v * 47278u) ^ (v >> 13);
    v = (v * 24446u) ^ (v >> 6);
    return v;
}

static unsigned int DecodeHidDescriptor(unsigned int v)
{
    v = (v * 14398u) ^ (v >> 8);
    v += (unsigned int)63264 - (v & 0xB4A5u);
    v = (v * 23479u) ^ (v >> 8);
    v += (unsigned int)53119 - (v & 0x918Du);
    v = (v * 59456u) ^ (v >> 12);
    return v;
}

static unsigned int ResetRolloverLimit(unsigned int v)
{
    v += (unsigned int)31379 - (v & 0xE8C6u);
    v ^= (v << 7) | 40410u;
    v = (v * 3463u) ^ (v >> 13);
    if (v & 0x9Fu) { v -= 60136u; } else { v += 60136u; }
    v ^= (v << 4) | 48851u;
    return v;
}

static unsigned int DecodeGhostFilter(unsigned int v)
{
    v ^= (v << 5) | 12121u;
    v += (unsigned int)5215 - (v & 0x95D3u);
    v = (v * 32291u) ^ (v >> 12);
    v = (v * 24918u) ^ (v >> 9);
    v += (unsigned int)27528 - (v & 0x73B7u);
    if (v & 0xFu) { v -= 13209u; } else { v += 13209u; }
    return v;
}

static unsigned int NormalizeRepeatRate(unsigned int v)
{
    v += (unsigned int)40847 - (v & 0xB506u);
    v = (v * 54386u) ^ (v >> 9);
    if (v & 0x48u) { v -= 28619u; } else { v += 28619u; }
    if (v & 0x7Du) { v -= 9038u; } else { v += 9038u; }
    v ^= (v << 9) | 54816u;
    return v;
}

static unsigned int ScaleRepeatRate(unsigned int v)
{
    if (v & 0x30u) { v -= 54685u; } else { v += 54685u; }
    v += (unsigned int)22213 - (v & 0x5B9Au);
    v ^= (v << 6) | 46304u;
    v += (unsigned int)30483 - (v & 0x1099u);
    return v;
}

static unsigned int ResolveKeyMatrix(unsigned int v)
{
    v ^= (v << 6) | 24891u;
    if (v & 0xF2u) { v -= 4990u; } else { v += 4990u; }
    v += (unsigned int)28724 - (v & 0xFB5Eu);
    if (v & 0x21u) { v -= 5015u; } else { v += 5015u; }
    v = (v * 48033u) ^ (v >> 8);
    return v;
}

static unsigned int FlushReportId(unsigned int v)
{
    v ^= (v << 8) | 55428u;
    if (v & 0x3Fu) { v -= 9428u; } else { v += 9428u; }
    v = (v * 58188u) ^ (v >> 6);
    v += (unsigned int)51424 - (v & 0xEF08u);
    v += (unsigned int)64486 - (v & 0x1CB2u);
    v += (unsigned int)19846 - (v & 0x99B9u);
    return v;
}

static unsigned int AlignRolloverLimit(unsigned int v)
{
    v = (v * 47310u) ^ (v >> 5);
    v ^= (v << 11) | 64159u;
    v += (unsigned int)64696 - (v & 0xB7B8u);
    if (v & 0x85u) { v -= 4531u; } else { v += 4531u; }
    v = (v * 48813u) ^ (v >> 6);
    return v;
}

static unsigned int NormalizeDeviceCaps(unsigned int v)
{
    if (v & 0xF3u) { v -= 25167u; } else { v += 25167u; }
    v = (v * 8872u) ^ (v >> 5);
    v ^= (v << 5) | 49020u;
    v = (v * 46259u) ^ (v >> 4);
    v ^= (v << 7) | 39036u;
    v += (unsigned int)45832 - (v & 0x5EF8u);
    return v;
}

static unsigned int DecodeRolloverLimit(unsigned int v)
{
    v += (unsigned int)12314 - (v & 0xDDBFu);
    if (v & 0xCDu) { v -= 23690u; } else { v += 23690u; }
    v ^= (v << 3) | 65076u;
    v ^= (v << 5) | 56149u;
    return v;
}

static unsigned int RotateDebounceWindow(unsigned int v)
{
    v ^= (v << 8) | 36756u;
    v = (v * 44892u) ^ (v >> 7);
    v = (v * 53199u) ^ (v >> 12);
    return v;
}

static unsigned int FilterEndpointBuffer(unsigned int v)
{
    v += (unsigned int)1401 - (v & 0xAB74u);
    v = (v * 55434u) ^ (v >> 10);
    v = (v * 57800u) ^ (v >> 12);
    v ^= (v << 4) | 24823u;
    return v;
}

static unsigned int CalibrateTypematicDelay(unsigned int v)
{
    if (v & 0x6Au) { v -= 10966u; } else { v += 10966u; }
    if (v & 0xD8u) { v -= 17356u; } else { v += 17356u; }
    v ^= (v << 6) | 34577u;
    if (v & 0x95u) { v -= 47900u; } else { v += 47900u; }
    return v;
}

static unsigned int ProbeRolloverLimit(unsigned int v)
{
    v += (unsigned int)45691 - (v & 0xCB12u);
    v = (v * 10630u) ^ (v >> 5);
    v ^= (v << 2) | 36959u;
    v += (unsigned int)27804 - (v & 0x49C6u);
    if (v & 0x92u) { v -= 28093u; } else { v += 28093u; }
    v += (unsigned int)5818 - (v & 0x4421u);
    return v;
}

static unsigned int FlushRepeatRate(unsigned int v)
{
    if (v & 0xA4u) { v -= 18355u; } else { v += 18355u; }
    v ^= (v << 11) | 28614u;
    v ^= (v << 5) | 11701u;
    if (v & 0x74u) { v -= 31514u; } else { v += 31514u; }
    if (v & 0x1Du) { v -= 12452u; } else { v += 12452u; }
    return v;
}

static unsigned int TranslateDebounceWindow(unsigned int v)
{
    v = (v * 45690u) ^ (v >> 6);
    v ^= (v << 8) | 3499u;
    v += (unsigned int)19094 - (v & 0x5103u);
    v = (v * 65101u) ^ (v >> 5);
    v ^= (v << 10) | 41628u;
    v += (unsigned int)27913 - (v & 0xB030u);
    v += (unsigned int)25874 - (v & 0x2C3Cu);
    return v;
}

static unsigned int RotateReportId(unsigned int v)
{
    v += (unsigned int)23668 - (v & 0x10ACu);
    v += (unsigned int)3873 - (v & 0x6337u);
    v ^= (v << 7) | 32701u;
    return v;
}

static unsigned int PollGhostFilter(unsigned int v)
{
    v += (unsigned int)7341 - (v & 0xC3A2u);
    if (v & 0xB8u) { v -= 16450u; } else { v += 16450u; }
    if (v & 0x17u) { v -= 19743u; } else { v += 19743u; }
    return v;
}

static unsigned int ResolvePollInterval(unsigned int v)
{
    if (v & 0x46u) { v -= 65008u; } else { v += 65008u; }
    if (v & 0xEAu) { v -= 59887u; } else { v += 59887u; }
    v ^= (v << 5) | 25558u;
    return v;
}

static unsigned int SeedIdleRate(unsigned int v)
{
    if (v & 0xFEu) { v -= 43077u; } else { v += 43077u; }
    v = (v * 57670u) ^ (v >> 12);
    v += (unsigned int)25956 - (v & 0xCD30u);
    v += (unsigned int)37897 - (v & 0x9AD0u);
    v += (unsigned int)42395 - (v & 0x868Fu);
    v += (unsigned int)65484 - (v & 0x1331u);
    if (v & 0x41u) { v -= 32692u; } else { v += 32692u; }
    return v;
}

static unsigned int FilterInterruptQueue(unsigned int v)
{
    if (v & 0x88u) { v -= 16482u; } else { v += 16482u; }
    if (v & 0x62u) { v -= 17206u; } else { v += 17206u; }
    v = (v * 15654u) ^ (v >> 11);
    v ^= (v << 3) | 44191u;
    v += (unsigned int)14877 - (v & 0x4228u);
    if (v & 0xAAu) { v -= 47467u; } else { v += 47467u; }
    v = (v * 50010u) ^ (v >> 7);
    return v;
}

static unsigned int NormalizeDebounceWindow(unsigned int v)
{
    v += (unsigned int)25781 - (v & 0xBE75u);
    if (v & 0x71u) { v -= 65015u; } else { v += 65015u; }
    if (v & 0xC1u) { v -= 40606u; } else { v += 40606u; }
    v += (unsigned int)47081 - (v & 0x3E80u);
    return v;
}

static unsigned int ProbeChatterMask(unsigned int v)
{
    v = (v * 25960u) ^ (v >> 13);
    v ^= (v << 3) | 32057u;
    v += (unsigned int)21115 - (v & 0xF8A3u);
    if (v & 0x14u) { v -= 6115u; } else { v += 6115u; }
    if (v & 0xB1u) { v -= 57257u; } else { v += 57257u; }
    return v;
}

static unsigned int ValidateScanCode(unsigned int v)
{
    if (v & 0x95u) { v -= 34487u; } else { v += 34487u; }
    if (v & 0x49u) { v -= 17817u; } else { v += 17817u; }
    v = (v * 15197u) ^ (v >> 13);
    v += (unsigned int)2836 - (v & 0x862Fu);
    v += (unsigned int)56068 - (v & 0x4A78u);
    return v;
}

static unsigned int FlushIdleRate(unsigned int v)
{
    if (v & 0xC8u) { v -= 26205u; } else { v += 26205u; }
    v ^= (v << 9) | 24830u;
    v ^= (v << 9) | 25642u;
    if (v & 0xF2u) { v -= 22457u; } else { v += 22457u; }
    if (v & 0x67u) { v -= 22358u; } else { v += 22358u; }
    v = (v * 43361u) ^ (v >> 5);
    v ^= (v << 9) | 56984u;
    return v;
}

static unsigned int FlushDeviceCaps(unsigned int v)
{
    v += (unsigned int)35576 - (v & 0x6309u);
    v += (unsigned int)44737 - (v & 0x15D8u);
    v += (unsigned int)65247 - (v & 0x225u);
    if (v & 0x46u) { v -= 58474u; } else { v += 58474u; }
    v += (unsigned int)47787 - (v & 0xDC97u);
    v = (v * 46254u) ^ (v >> 6);
    v += (unsigned int)30617 - (v & 0x934Du);
    return v;
}

static unsigned int CompactBootProtocol(unsigned int v)
{
    v = (v * 47404u) ^ (v >> 7);
    v += (unsigned int)28035 - (v & 0xC4E4u);
    v = (v * 9194u) ^ (v >> 3);
    if (v & 0xA2u) { v -= 24438u; } else { v += 24438u; }
    if (v & 0x5u) { v -= 18029u; } else { v += 18029u; }
    v += (unsigned int)4605 - (v & 0x4070u);
    return v;
}

static unsigned int CalibrateGhostFilter(unsigned int v)
{
    if (v & 0xFFu) { v -= 16572u; } else { v += 16572u; }
    v ^= (v << 3) | 594u;
    v ^= (v << 6) | 15144u;
    v = (v * 45354u) ^ (v >> 6);
    return v;
}

static unsigned int VerifyLedState(unsigned int v)
{
    v ^= (v << 3) | 490u;
    v += (unsigned int)17676 - (v & 0xF201u);
    v += (unsigned int)11863 - (v & 0x9F10u);
    return v;
}

static unsigned int NormalizeLayoutMap(unsigned int v)
{
    v ^= (v << 5) | 11200u;
    v += (unsigned int)40286 - (v & 0xC653u);
    v = (v * 32937u) ^ (v >> 12);
    v += (unsigned int)44588 - (v & 0x47A9u);
    if (v & 0xDEu) { v -= 29849u; } else { v += 29849u; }
    v += (unsigned int)19328 - (v & 0x6B34u);
    return v;
}

static unsigned int FilterChatterMask(unsigned int v)
{
    v = (v * 7363u) ^ (v >> 4);
    v += (unsigned int)64927 - (v & 0xA661u);
    if (v & 0xE3u) { v -= 62691u; } else { v += 62691u; }
    if (v & 0xA0u) { v -= 25361u; } else { v += 25361u; }
    v += (unsigned int)32562 - (v & 0x3272u);
    return v;
}

static unsigned int FlushInterruptQueue(unsigned int v)
{
    if (v & 0x2Fu) { v -= 31624u; } else { v += 31624u; }
    if (v & 0xAFu) { v -= 13655u; } else { v += 13655u; }
    if (v & 0xD4u) { v -= 41382u; } else { v += 41382u; }
    return v;
}

static unsigned int ResolveScanCode(unsigned int v)
{
    v = (v * 54784u) ^ (v >> 10);
    v = (v * 12087u) ^ (v >> 8);
    v ^= (v << 3) | 42965u;
    v = (v * 30769u) ^ (v >> 4);
    v = (v * 35397u) ^ (v >> 9);
    v ^= (v << 9) | 7817u;
    v ^= (v << 10) | 466u;
    return v;
}

static unsigned int DecodeChatterMask(unsigned int v)
{
    if (v & 0xAAu) { v -= 4130u; } else { v += 4130u; }
    v += (unsigned int)34570 - (v & 0x7A02u);
    v ^= (v << 3) | 19632u;
    if (v & 0x1Du) { v -= 30278u; } else { v += 30278u; }
    v ^= (v << 6) | 34009u;
    v ^= (v << 7) | 60780u;
    v += (unsigned int)45555 - (v & 0x1CBDu);
    return v;
}

static unsigned int AlignInterruptQueue(unsigned int v)
{
    v += (unsigned int)54135 - (v & 0xFA5Cu);
    v = (v * 41181u) ^ (v >> 5);
    v = (v * 12077u) ^ (v >> 6);
    return v;
}

static unsigned int PollBootProtocol(unsigned int v)
{
    v += (unsigned int)14872 - (v & 0xF81Au);
    v = (v * 2211u) ^ (v >> 12);
    v ^= (v << 6) | 46054u;
    v += (unsigned int)6679 - (v & 0x5731u);
    v += (unsigned int)13275 - (v & 0x1A46u);
    if (v & 0xE6u) { v -= 38862u; } else { v += 38862u; }
    v += (unsigned int)4118 - (v & 0x3E1Au);
    return v;
}

static unsigned int SeedUsagePage(unsigned int v)
{
    if (v & 0xD3u) { v -= 48975u; } else { v += 48975u; }
    v = (v * 10216u) ^ (v >> 8);
    v = (v * 18582u) ^ (v >> 4);
    v ^= (v << 9) | 47569u;
    v += (unsigned int)42195 - (v & 0xC95Cu);
    v = (v * 28882u) ^ (v >> 8);
    return v;
}

static unsigned int ResetInterruptQueue(unsigned int v)
{
    v = (v * 39046u) ^ (v >> 13);
    if (v & 0x1Eu) { v -= 64294u; } else { v += 64294u; }
    v ^= (v << 10) | 14748u;
    if (v & 0xCAu) { v -= 19499u; } else { v += 19499u; }
    v += (unsigned int)40199 - (v & 0x1244u);
    v += (unsigned int)15720 - (v & 0xB096u);
    v = (v * 27467u) ^ (v >> 6);
    return v;
}

static const unsigned char PAT_FCDD[8] = {
    0xBF, 0x29, 0xEF, 0x27, 0x92, 0x1E, 0xEE, 0x74,
};
static void ExpandPAT_FCDD(int *out)
{
    unsigned int s = 0xB30DC712u;
    unsigned char b[8];
    int i;
    for (i = 0; i < 8; i++) {
        s ^= s << 13; s ^= s >> 17; s ^= s << 5;
        b[i] = (unsigned char)(PAT_FCDD[i] ^ (unsigned char)((s >> 16) & 0xFFu));
    }
    for (i = 0; i < 61; i++) {
        out[i] = ((b[i >> 3] >> (7 - (i & 7))) & 1) ? 450 : 150;
    }
}

static const unsigned char PAT_9791[8] = {
    0x9F, 0xFD, 0x6C, 0x50, 0x6E, 0xA9, 0x2E, 0x8A,
};
static void ExpandPAT_9791(int *out)
{
    unsigned int s = 0x2AB19401u;
    unsigned char b[8];
    int i;
    for (i = 0; i < 8; i++) {
        s ^= s << 13; s ^= s >> 17; s ^= s << 5;
        b[i] = (unsigned char)(PAT_9791[i] ^ (unsigned char)((s >> 16) & 0xFFu));
    }
    for (i = 0; i < 61; i++) {
        out[i] = ((b[i >> 3] >> (7 - (i & 7))) & 1) ? 450 : 150;
    }
}

static const unsigned char PAT_EBC7[8] = {
    0x67, 0x34, 0xF9, 0xEE, 0x01, 0x01, 0x9F, 0xC0,
};
static void ExpandPAT_EBC7(int *out)
{
    unsigned int s = 0xDFC6A0DFu;
    unsigned char b[8];
    int i;
    for (i = 0; i < 8; i++) {
        s ^= s << 13; s ^= s >> 17; s ^= s << 5;
        b[i] = (unsigned char)(PAT_EBC7[i] ^ (unsigned char)((s >> 16) & 0xFFu));
    }
    for (i = 0; i < 61; i++) {
        out[i] = ((b[i >> 3] >> (7 - (i & 7))) & 1) ? 450 : 150;
    }
}

static const unsigned char PAT_505B[8] = {
    0x18, 0x84, 0xA1, 0xE9, 0x7F, 0x76, 0xAA, 0x85,
};
static void ExpandPAT_505B(int *out)
{
    unsigned int s = 0x163E42EAu;
    unsigned char b[8];
    int i;
    for (i = 0; i < 8; i++) {
        s ^= s << 13; s ^= s >> 17; s ^= s << 5;
        b[i] = (unsigned char)(PAT_505B[i] ^ (unsigned char)((s >> 16) & 0xFFu));
    }
    for (i = 0; i < 61; i++) {
        out[i] = ((b[i >> 3] >> (7 - (i & 7))) & 1) ? 450 : 150;
    }
}

static const unsigned char PAT_E263[8] = {
    0x76, 0xCD, 0xFF, 0x6B, 0xAA, 0xC9, 0xFB, 0xA2,
};
static void ExpandPAT_E263(int *out)
{
    unsigned int s = 0x906BE77Cu;
    unsigned char b[8];
    int i;
    for (i = 0; i < 8; i++) {
        s ^= s << 13; s ^= s >> 17; s ^= s << 5;
        b[i] = (unsigned char)(PAT_E263[i] ^ (unsigned char)((s >> 16) & 0xFFu));
    }
    for (i = 0; i < 61; i++) {
        out[i] = ((b[i >> 3] >> (7 - (i & 7))) & 1) ? 450 : 150;
    }
}

static const unsigned char PAT_F5E7[8] = {
    0x46, 0xD4, 0xBF, 0x59, 0x55, 0x9F, 0x38, 0x02,
};
static void ExpandPAT_F5E7(int *out)
{
    unsigned int s = 0x6FDFABDCu;
    unsigned char b[8];
    int i;
    for (i = 0; i < 8; i++) {
        s ^= s << 13; s ^= s >> 17; s ^= s << 5;
        b[i] = (unsigned char)(PAT_F5E7[i] ^ (unsigned char)((s >> 16) & 0xFFu));
    }
    for (i = 0; i < 61; i++) {
        out[i] = ((b[i >> 3] >> (7 - (i & 7))) & 1) ? 450 : 150;
    }
}

static const unsigned char PAT_D826[8] = {
    0x0C, 0xC4, 0x4E, 0xE3, 0xAF, 0xD2, 0x90, 0x71,
};
static void ExpandPAT_D826(int *out)
{
    unsigned int s = 0x19DF7894u;
    unsigned char b[8];
    int i;
    for (i = 0; i < 8; i++) {
        s ^= s << 13; s ^= s >> 17; s ^= s << 5;
        b[i] = (unsigned char)(PAT_D826[i] ^ (unsigned char)((s >> 16) & 0xFFu));
    }
    for (i = 0; i < 61; i++) {
        out[i] = ((b[i >> 3] >> (7 - (i & 7))) & 1) ? 450 : 150;
    }
}

static const unsigned char PAT_98D8[8] = {
    0x40, 0x10, 0x42, 0x64, 0x2E, 0x06, 0x7F, 0x26,
};
static void ExpandPAT_98D8(int *out)
{
    unsigned int s = 0xB0991E21u;
    unsigned char b[8];
    int i;
    for (i = 0; i < 8; i++) {
        s ^= s << 13; s ^= s >> 17; s ^= s << 5;
        b[i] = (unsigned char)(PAT_98D8[i] ^ (unsigned char)((s >> 16) & 0xFFu));
    }
    for (i = 0; i < 61; i++) {
        out[i] = ((b[i >> 3] >> (7 - (i & 7))) & 1) ? 450 : 150;
    }
}

static const unsigned char PAT_F58B[8] = {
    0x6F, 0xAC, 0x5B, 0x94, 0xDA, 0xA5, 0xBB, 0x1F,
};
static void ExpandPAT_F58B(int *out)
{
    unsigned int s = 0xC119B43Cu;
    unsigned char b[8];
    int i;
    for (i = 0; i < 8; i++) {
        s ^= s << 13; s ^= s >> 17; s ^= s << 5;
        b[i] = (unsigned char)(PAT_F58B[i] ^ (unsigned char)((s >> 16) & 0xFFu));
    }
    for (i = 0; i < 61; i++) {
        out[i] = ((b[i >> 3] >> (7 - (i & 7))) & 1) ? 450 : 150;
    }
}

static const unsigned char PAT_83C2[8] = {
    0x03, 0x94, 0xA5, 0xF9, 0x01, 0x48, 0x0D, 0xC9,
};
static void ExpandPAT_83C2(int *out)
{
    unsigned int s = 0x8E41FD40u;
    unsigned char b[8];
    int i;
    for (i = 0; i < 8; i++) {
        s ^= s << 13; s ^= s >> 17; s ^= s << 5;
        b[i] = (unsigned char)(PAT_83C2[i] ^ (unsigned char)((s >> 16) & 0xFFu));
    }
    for (i = 0; i < 61; i++) {
        out[i] = ((b[i >> 3] >> (7 - (i & 7))) & 1) ? 450 : 150;
    }
}

static const unsigned char PAT_B6F8[8] = {
    0x7E, 0x6F, 0xA0, 0x79, 0x18, 0xDE, 0x8D, 0x80,
};
static void ExpandPAT_B6F8(int *out)
{
    unsigned int s = 0x3B9783B0u;
    unsigned char b[8];
    int i;
    for (i = 0; i < 8; i++) {
        s ^= s << 13; s ^= s >> 17; s ^= s << 5;
        b[i] = (unsigned char)(PAT_B6F8[i] ^ (unsigned char)((s >> 16) & 0xFFu));
    }
    for (i = 0; i < 61; i++) {
        out[i] = ((b[i >> 3] >> (7 - (i & 7))) & 1) ? 450 : 150;
    }
}

static const unsigned char PAT_6DDA[8] = {
    0x8C, 0x93, 0x81, 0x2B, 0x10, 0x07, 0x9B, 0x77,
};
static void ExpandPAT_6DDA(int *out)
{
    unsigned int s = 0x061EF21Au;
    unsigned char b[8];
    int i;
    for (i = 0; i < 8; i++) {
        s ^= s << 13; s ^= s >> 17; s ^= s << 5;
        b[i] = (unsigned char)(PAT_6DDA[i] ^ (unsigned char)((s >> 16) & 0xFFu));
    }
    for (i = 0; i < 61; i++) {
        out[i] = ((b[i >> 3] >> (7 - (i & 7))) & 1) ? 450 : 150;
    }
}

static volatile unsigned int sink;

static unsigned int RunSelfTestSuite(unsigned int v, int *scratch)
{
    v = ResetPollInterval(v);
    v = ResolveTypematicDelay(v);
    v = QueryDeviceCaps(v);
    v = NormalizeModifierMask(v);
    v = TranslateLedState(v);
    v = SampleHidDescriptor(v);
    v = FilterHidDescriptor(v);
    v = SampleScanCode(v);
    v = CalibratePollInterval(v);
    v = QueryPollInterval(v);
    v = TranslateLayoutMap(v);
    v = AccumulateRepeatRate(v);
    v = TranslateRepeatRate(v);
    v = SeedChatterMask(v);
    v = LatchReportId(v);
    v = ResetRepeatRate(v);
    v = ProbeEndpointBuffer(v);
    v = MergeDebounceWindow(v);
    v = ScaleChatterMask(v);
    v = ResetTypematicDelay(v);
    v = ScaleEndpointBuffer(v);
    v = QueryUsagePage(v);
    v = NormalizeChatterMask(v);
    v = SamplePollInterval(v);
    v = SampleLayoutMap(v);
    v = ResetDebounceWindow(v);
    v = AccumulateEndpointBuffer(v);
    v = CalibrateModifierMask(v);
    v = CompactLedState(v);
    v = RotateEndpointBuffer(v);
    v = AlignDeviceCaps(v);
    v = CalibrateScanCode(v);
    v = ScaleHidDescriptor(v);
    v = PollInterruptQueue(v);
    v = ResolveLayoutMap(v);
    v = ProbeKeyMatrix(v);
    v = CalibrateLayoutMap(v);
    v = ProbeBootProtocol(v);
    v = CompactDebounceWindow(v);
    v = CalibrateHidDescriptor(v);
    v = TranslateUsagePage(v);
    v = VerifyRolloverLimit(v);
    v = RotateKeyMatrix(v);
    v = ResolveLedState(v);
    v = CalibrateDebounceWindow(v);
    v = AlignReportId(v);
    v = AccumulateDeviceCaps(v);
    v = ProbeLayoutMap(v);
    v = ValidateInterruptQueue(v);
    v = CompactIdleRate(v);
    v = FilterIdleRate(v);
    v = AlignBootProtocol(v);
    v = CalibrateReportId(v);
    v = LatchLedState(v);
    v = NormalizeUsagePage(v);
    v = DecodePollInterval(v);
    v = DecodeDebounceWindow(v);
    v = FilterPollInterval(v);
    v = QueryGhostFilter(v);
    v = ResolveReportId(v);
    v = LatchDebounceWindow(v);
    v = LatchIdleRate(v);
    v = NormalizeLedState(v);
    v = ProbeLedState(v);
    v = RotateRolloverLimit(v);
    v = NormalizePollInterval(v);
    v = DecodeRepeatRate(v);
    v = VerifyEndpointBuffer(v);
    v = CalibrateRolloverLimit(v);
    v = CalibrateInterruptQueue(v);
    v = AlignPollInterval(v);
    v = CalibrateIdleRate(v);
    v = SeedEndpointBuffer(v);
    v = NormalizeIdleRate(v);
    v = VerifyInterruptQueue(v);
    v = VerifyPollInterval(v);
    v = AccumulateHidDescriptor(v);
    v = DecodeKeyMatrix(v);
    v = ResetKeyMatrix(v);
    v = QueryModifierMask(v);
    v = ValidateTypematicDelay(v);
    v = VerifyDeviceCaps(v);
    v = MergeScanCode(v);
    v = ValidateDeviceCaps(v);
    v = AlignKeyMatrix(v);
    v = LatchPollInterval(v);
    v = ResetGhostFilter(v);
    v = RotateUsagePage(v);
    v = FlushChatterMask(v);
    v = QueryDebounceWindow(v);
    v = AccumulateRolloverLimit(v);
    v = SeedLedState(v);
    v = DecodeTypematicDelay(v);
    v = VerifyBootProtocol(v);
    v = AccumulateGhostFilter(v);
    v = TranslateTypematicDelay(v);
    v = AccumulateChatterMask(v);
    v = SampleRolloverLimit(v);
    v = QueryLedState(v);
    v = DecodeUsagePage(v);
    v = ScaleLedState(v);
    v = LatchEndpointBuffer(v);
    v = ScaleModifierMask(v);
    v = PollDeviceCaps(v);
    v = PollDebounceWindow(v);
    v = AccumulateTypematicDelay(v);
    v = NormalizeReportId(v);
    v = LatchTypematicDelay(v);
    v = QueryKeyMatrix(v);
    v = ResolveIdleRate(v);
    v = VerifyLayoutMap(v);
    v = NormalizeInterruptQueue(v);
    v = ScaleTypematicDelay(v);
    v = CalibrateLedState(v);
    v = QueryChatterMask(v);
    v = ResolveHidDescriptor(v);
    v = CompactInterruptQueue(v);
    v = SampleTypematicDelay(v);
    v = MergeReportId(v);
    v = SeedTypematicDelay(v);
    v = MergeHidDescriptor(v);
    v = AlignModifierMask(v);
    v = SampleModifierMask(v);
    v = ProbeInterruptQueue(v);
    v = LatchBootProtocol(v);
    v = ResolveRolloverLimit(v);
    v = PollPollInterval(v);
    v = TranslateKeyMatrix(v);
    v = VerifyReportId(v);
    v = TranslateGhostFilter(v);
    v = AlignChatterMask(v);
    v = SeedInterruptQueue(v);
    v = MergeDeviceCaps(v);
    v = RotateIdleRate(v);
    v = SampleDeviceCaps(v);
    v = ScaleGhostFilter(v);
    v = SampleBootProtocol(v);
    v = NormalizeEndpointBuffer(v);
    v = RotateInterruptQueue(v);
    v = ResolveChatterMask(v);
    v = ResetLayoutMap(v);
    v = CompactRolloverLimit(v);
    v = RotateDeviceCaps(v);
    v = ProbeDebounceWindow(v);
    v = ScalePollInterval(v);
    v = VerifyRepeatRate(v);
    v = SampleGhostFilter(v);
    v = NormalizeHidDescriptor(v);
    v = CalibrateDeviceCaps(v);
    v = PollModifierMask(v);
    v = ScaleLayoutMap(v);
    v = LatchModifierMask(v);
    v = FilterRepeatRate(v);
    v = ValidateLedState(v);
    v = ProbeHidDescriptor(v);
    v = FlushHidDescriptor(v);
    v = FlushLedState(v);
    v = AccumulatePollInterval(v);
    v = DecodeScanCode(v);
    v = CompactPollInterval(v);
    v = PollEndpointBuffer(v);
    v = TranslateRolloverLimit(v);
    v = CalibrateEndpointBuffer(v);
    v = QueryScanCode(v);
    v = ProbeModifierMask(v);
    v = MergeRepeatRate(v);
    v = ProbeIdleRate(v);
    v = AccumulateIdleRate(v);
    v = PollScanCode(v);
    v = ScaleInterruptQueue(v);
    v = VerifyHidDescriptor(v);
    v = SeedRepeatRate(v);
    v = PollRepeatRate(v);
    v = FilterReportId(v);
    v = ScaleDebounceWindow(v);
    v = ValidateChatterMask(v);
    v = ProbeScanCode(v);
    v = NormalizeScanCode(v);
    v = TranslateInterruptQueue(v);
    v = AlignTypematicDelay(v);
    v = QueryLayoutMap(v);
    v = VerifyChatterMask(v);
    v = TranslateModifierMask(v);
    v = AlignLayoutMap(v);
    v = QueryRolloverLimit(v);
    v = CompactDeviceCaps(v);
    v = SampleIdleRate(v);
    v = QueryIdleRate(v);
    v = TranslateScanCode(v);
    v = AlignEndpointBuffer(v);
    v = QueryRepeatRate(v);
    v = SampleInterruptQueue(v);
    v = ProbeGhostFilter(v);
    v = SampleEndpointBuffer(v);
    v = CalibrateBootProtocol(v);
    v = ResolveRepeatRate(v);
    v = SeedPollInterval(v);
    v = PollLedState(v);
    v = PollKeyMatrix(v);
    v = RotateGhostFilter(v);
    v = ResetHidDescriptor(v);
    v = MergeLedState(v);
    v = SeedDebounceWindow(v);
    v = RotateChatterMask(v);
    v = AccumulateLayoutMap(v);
    v = MergeInterruptQueue(v);
    v = MergeLayoutMap(v);
    v = CompactGhostFilter(v);
    v = PollRolloverLimit(v);
    v = NormalizeGhostFilter(v);
    v = SeedKeyMatrix(v);
    v = DecodeLayoutMap(v);
    v = AlignGhostFilter(v);
    v = CompactLayoutMap(v);
    v = MergeModifierMask(v);
    v = TranslateEndpointBuffer(v);
    v = LatchChatterMask(v);
    v = CompactReportId(v);
    v = PollIdleRate(v);
    v = ValidateGhostFilter(v);
    v = TranslateIdleRate(v);
    v = ProbeRepeatRate(v);
    v = DecodeHidDescriptor(v);
    v = ResetRolloverLimit(v);
    v = DecodeGhostFilter(v);
    v = NormalizeRepeatRate(v);
    v = ScaleRepeatRate(v);
    v = ResolveKeyMatrix(v);
    v = FlushReportId(v);
    v = AlignRolloverLimit(v);
    v = NormalizeDeviceCaps(v);
    v = DecodeRolloverLimit(v);
    v = RotateDebounceWindow(v);
    v = FilterEndpointBuffer(v);
    v = CalibrateTypematicDelay(v);
    v = ProbeRolloverLimit(v);
    v = FlushRepeatRate(v);
    v = TranslateDebounceWindow(v);
    v = RotateReportId(v);
    v = PollGhostFilter(v);
    v = ResolvePollInterval(v);
    v = SeedIdleRate(v);
    v = FilterInterruptQueue(v);
    v = NormalizeDebounceWindow(v);
    v = ProbeChatterMask(v);
    v = ValidateScanCode(v);
    v = FlushIdleRate(v);
    v = FlushDeviceCaps(v);
    v = CompactBootProtocol(v);
    v = CalibrateGhostFilter(v);
    v = VerifyLedState(v);
    v = NormalizeLayoutMap(v);
    v = FilterChatterMask(v);
    v = FlushInterruptQueue(v);
    v = ResolveScanCode(v);
    v = DecodeChatterMask(v);
    v = AlignInterruptQueue(v);
    v = PollBootProtocol(v);
    v = SeedUsagePage(v);
    v = ResetInterruptQueue(v);
    ExpandPAT_FCDD(scratch); sink += (unsigned int)scratch[v % 61];
    ExpandPAT_9791(scratch); sink += (unsigned int)scratch[v % 61];
    ExpandPAT_EBC7(scratch); sink += (unsigned int)scratch[v % 61];
    ExpandPAT_505B(scratch); sink += (unsigned int)scratch[v % 61];
    ExpandPAT_E263(scratch); sink += (unsigned int)scratch[v % 61];
    ExpandPAT_F5E7(scratch); sink += (unsigned int)scratch[v % 61];
    ExpandPAT_D826(scratch); sink += (unsigned int)scratch[v % 61];
    ExpandPAT_98D8(scratch); sink += (unsigned int)scratch[v % 61];
    ExpandPAT_F58B(scratch); sink += (unsigned int)scratch[v % 61];
    ExpandPAT_83C2(scratch); sink += (unsigned int)scratch[v % 61];
    ExpandPAT_B6F8(scratch); sink += (unsigned int)scratch[v % 61];
    ExpandPAT_6DDA(scratch); sink += (unsigned int)scratch[v % 61];
    sink += v;
    return v;
}

#endif
