package tn.securinets.ctf

import tn.securinets.ctf.challenges.finalcountdown.FinalCountdownActivity
import tn.securinets.ctf.challenges.license.LicenseActivity
import tn.securinets.ctf.challenges.whatremains.WhatRemainsActivity
import tn.securinets.ctf.challenges.facetoface.FaceToFaceActivity
import tn.securinets.ctf.challenges.forgedpapers.ForgedPapersActivity
import tn.securinets.ctf.challenges.l0gin.L0gInActivity
import tn.securinets.ctf.challenges.warmup.WarmupActivity
import tn.securinets.ctf.challenges.firstcontact.FirstContactActivity
import tn.securinets.ctf.challenges.echoes.EchoesActivity
import tn.securinets.ctf.challenges.wideopen.WideOpenActivity
import tn.securinets.ctf.challenges.openlines.OpenLinesActivity
import tn.securinets.ctf.challenges.plainsight.PlainSightActivity
import tn.securinets.ctf.challenges.strangers.StrangersActivity
import tn.securinets.ctf.challenges.pinned.PinnedActivity
import tn.securinets.ctf.challenges.nobodycalled.NobodyCalledActivity
import tn.securinets.ctf.challenges.dor.DorActivity

object ChallengeRouter {

    fun activityFor(id: Int): Class<*>? = when (id) {
        0 -> PlainSightActivity::class.java
        1 -> FirstContactActivity::class.java
        2 -> WarmupActivity::class.java
        3 -> EchoesActivity::class.java
        4 -> WhatRemainsActivity::class.java
        5 -> L0gInActivity::class.java
        6 -> null
        8 -> FaceToFaceActivity::class.java
        9 -> OpenLinesActivity::class.java
        10 -> DorActivity::class.java
        11 -> PinnedActivity::class.java
        12 -> ForgedPapersActivity::class.java
        13 -> WideOpenActivity::class.java
        14 -> StrangersActivity::class.java
        15 -> LicenseActivity::class.java
        16 -> NobodyCalledActivity::class.java
        17 -> FinalCountdownActivity::class.java
        else -> null
    }
}
