// Stage clear bonus labels of TH02's MAIN.EXE, defined in the bonus TU.
// Not `const`: TCC would move `const` character arrays into the literal pool.

// ZUN bug: Why is that space here? None of the other labels even attempt to
// be centered.
shiftjis_t BONUS_RANK[] = " 難易度";
shiftjis_t BONUS_PLAYPERF[] = "ステージ";
shiftjis_t BONUS_BOMBS[] = "ボム";
shiftjis_t BONUS_LIVES[] = "ミス";
shiftjis_t BONUS_START_BOMBS[] = "靈撃初期数";
shiftjis_t BONUS_START_LIVES[] = "靈夢初期数";
shiftjis_t BONUS_POINT[] = "点";
shiftjis_t BONUS_TIMES[] = "×";
shiftjis_t BONUS_EQUALS[] = "＝";
shiftjis_t BONUS_EXTRA_CLEAR[] = "クリア";
shiftjis_t BONUS_EXTRA_LIVES[] = "ミス回数";
shiftjis_t BONUS_EXTRA_BOMBS[] = "ボム回数";
shiftjis_t BONUS_EXTRA_SIGMA_FRAMES[] = "クリアタイム";
