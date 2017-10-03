// works after 49 days
DWORD GetTickCountSpan(DWORD dwTickStart, DWORD dwTickEnd)
{
	const DWORD dwCurTime = GetTickCount();
	return dwTickEnd>dwTickStart ? dwTickEnd-dwTickStart : 0xFFFFFFFF-dwTickStart + dwTickEnd + 1;
}
