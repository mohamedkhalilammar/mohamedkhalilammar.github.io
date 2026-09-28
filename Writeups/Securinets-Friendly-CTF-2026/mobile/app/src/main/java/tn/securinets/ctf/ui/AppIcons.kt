package tn.securinets.ctf.ui

import androidx.compose.foundation.Canvas
import androidx.compose.foundation.layout.size
import androidx.compose.runtime.Composable
import androidx.compose.ui.Modifier
import androidx.compose.ui.geometry.CornerRadius
import androidx.compose.ui.geometry.Offset
import androidx.compose.ui.geometry.Size
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.graphics.Path
import androidx.compose.ui.graphics.StrokeCap
import androidx.compose.ui.graphics.StrokeJoin
import androidx.compose.ui.graphics.drawscope.DrawScope
import androidx.compose.ui.graphics.drawscope.Stroke
import androidx.compose.ui.unit.Dp
import androidx.compose.ui.unit.dp

enum class AppIcon {
    Person, Persons, Shield, Lock, Unlock, Key, Fingerprint,
    Bell, ChevronRight, ChevronDown, ArrowBack, Search, More, Menu,
    Card, Wallet, Document, Folder, Cloud, Upload, Download, Refresh,
    Check, Close, Plus, Mail, Phone, Globe, Clock, Warning, Star,
    Send, Home, Chart, Eye, Terminal, Logout, Pin, Receipt, Sparkle,
}

@Composable
fun Glyph(
    icon: AppIcon,
    modifier: Modifier = Modifier,
    size: Dp = 20.dp,
    tint: Color = Color.White,
    weight: Float = 1f,
) {
    Canvas(modifier.size(size)) {
        val u = this.size.minDimension / 24f
        drawGlyph(icon, u, 1.8f * u * weight, tint)
    }
}

private fun DrawScope.ln(c: Color, w: Float, u: Float, x1: Float, y1: Float, x2: Float, y2: Float) =
    drawLine(c, Offset(x1 * u, y1 * u), Offset(x2 * u, y2 * u), w, StrokeCap.Round)

private fun DrawScope.circ(c: Color, w: Float, u: Float, cx: Float, cy: Float, r: Float) =
    drawCircle(c, r * u, Offset(cx * u, cy * u), style = Stroke(w))

private fun DrawScope.dot(c: Color, u: Float, cx: Float, cy: Float, r: Float) =
    drawCircle(c, r * u, Offset(cx * u, cy * u))

private fun DrawScope.box(
    c: Color, w: Float, u: Float,
    x: Float, y: Float, x2: Float, y2: Float, r: Float,
) = drawRoundRect(
    color = c,
    topLeft = Offset(x * u, y * u),
    size = Size((x2 - x) * u, (y2 - y) * u),
    cornerRadius = CornerRadius(r * u, r * u),
    style = Stroke(w, join = StrokeJoin.Round),
)

private fun DrawScope.solidBox(
    c: Color, u: Float,
    x: Float, y: Float, x2: Float, y2: Float, r: Float,
) = drawRoundRect(
    color = c,
    topLeft = Offset(x * u, y * u),
    size = Size((x2 - x) * u, (y2 - y) * u),
    cornerRadius = CornerRadius(r * u, r * u),
)

private fun DrawScope.arc(
    c: Color, w: Float, u: Float,
    x: Float, y: Float, x2: Float, y2: Float,
    start: Float, sweep: Float,
) = drawArc(
    color = c,
    startAngle = start,
    sweepAngle = sweep,
    useCenter = false,
    topLeft = Offset(x * u, y * u),
    size = Size((x2 - x) * u, (y2 - y) * u),
    style = Stroke(w, cap = StrokeCap.Round),
)

private fun DrawScope.poly(c: Color, w: Float, u: Float, close: Boolean, vararg pts: Float) {
    val path = Path().apply {
        moveTo(pts[0] * u, pts[1] * u)
        var i = 2
        while (i < pts.size) {
            lineTo(pts[i] * u, pts[i + 1] * u)
            i += 2
        }
        if (close) close()
    }
    drawPath(path, c, style = Stroke(w, cap = StrokeCap.Round, join = StrokeJoin.Round))
}

private fun DrawScope.drawGlyph(icon: AppIcon, u: Float, w: Float, c: Color) {
    when (icon) {
        AppIcon.Person -> {
            circ(c, w, u, 12f, 8f, 3.7f)
            arc(c, w, u, 4.6f, 13.4f, 19.4f, 26.4f, 180f, 180f)
        }
        AppIcon.Persons -> {
            circ(c, w, u, 9.5f, 8f, 3.2f)
            arc(c, w, u, 3f, 13f, 16f, 24f, 180f, 180f)
            arc(c, w, u, 14.5f, 4.9f, 21.5f, 11.9f, 300f, 120f)
            arc(c, w, u, 14f, 13f, 23f, 22f, 250f, 70f)
        }
        AppIcon.Shield -> poly(
            c, w, u, true,
            12f, 2.8f, 20f, 6.2f, 20f, 11.6f, 16.6f, 18.2f, 12f, 21.2f,
            7.4f, 18.2f, 4f, 11.6f, 4f, 6.2f,
        )
        AppIcon.Lock -> {
            box(c, w, u, 4.8f, 10.4f, 19.2f, 20.8f, 2.8f)
            arc(c, w, u, 7.6f, 4f, 16.4f, 12.8f, 180f, 180f)
            dot(c, u, 12f, 15.6f, 1.5f)
        }
        AppIcon.Unlock -> {
            box(c, w, u, 4.8f, 10.4f, 19.2f, 20.8f, 2.8f)
            arc(c, w, u, 11.2f, 4f, 20f, 12.8f, 180f, 140f)
            dot(c, u, 12f, 15.6f, 1.5f)
        }
        AppIcon.Key -> {
            circ(c, w, u, 8f, 8.6f, 4.2f)
            ln(c, w, u, 10.9f, 11.6f, 20f, 20.6f)
            ln(c, w, u, 17.4f, 18f, 15.2f, 20.2f)
        }
        AppIcon.Fingerprint -> {
            arc(c, w, u, 4f, 4f, 20f, 20f, 200f, 140f)
            arc(c, w, u, 6.6f, 6.6f, 17.4f, 19.4f, 195f, 150f)
            arc(c, w, u, 9.2f, 9.2f, 14.8f, 18.8f, 190f, 160f)
            ln(c, w, u, 12f, 12.4f, 12f, 18.6f)
        }
        AppIcon.Bell -> {
            poly(
                c, w, u, false,
                4.4f, 17.4f, 6.2f, 15.2f, 6.2f, 10.6f,
            )
            arc(c, w, u, 6.2f, 4.6f, 17.8f, 16.2f, 180f, 180f)
            poly(
                c, w, u, false,
                17.8f, 10.6f, 17.8f, 15.2f, 19.6f, 17.4f, 4.4f, 17.4f,
            )
            arc(c, w, u, 9.6f, 16.4f, 14.4f, 21.2f, 0f, 180f)
        }
        AppIcon.ChevronRight -> poly(c, w, u, false, 9.5f, 5.2f, 16.2f, 12f, 9.5f, 18.8f)
        AppIcon.ChevronDown -> poly(c, w, u, false, 5.2f, 9.5f, 12f, 16.2f, 18.8f, 9.5f)
        AppIcon.ArrowBack -> {
            ln(c, w, u, 20f, 12f, 4.4f, 12f)
            poly(c, w, u, false, 10.8f, 5.2f, 4f, 12f, 10.8f, 18.8f)
        }
        AppIcon.Search -> {
            circ(c, w, u, 10.4f, 10.4f, 6f)
            ln(c, w, u, 14.9f, 14.9f, 20.4f, 20.4f)
        }
        AppIcon.More -> {
            dot(c, u, 12f, 5.4f, 1.6f)
            dot(c, u, 12f, 12f, 1.6f)
            dot(c, u, 12f, 18.6f, 1.6f)
        }
        AppIcon.Menu -> {
            ln(c, w, u, 4f, 7f, 20f, 7f)
            ln(c, w, u, 4f, 12f, 20f, 12f)
            ln(c, w, u, 4f, 17f, 14f, 17f)
        }
        AppIcon.Card -> {
            box(c, w, u, 2.6f, 5.4f, 21.4f, 18.6f, 2.6f)
            solidBox(c, u, 2.6f, 9f, 21.4f, 11.4f, 0f)
            ln(c, w, u, 6f, 15f, 10.5f, 15f)
        }
        AppIcon.Wallet -> {
            box(c, w, u, 3f, 5.6f, 21f, 19.4f, 3f)
            ln(c, w, u, 3f, 10.4f, 21f, 10.4f)
            dot(c, u, 17.2f, 14.9f, 1.5f)
        }
        AppIcon.Receipt -> {
            poly(
                c, w, u, true,
                5f, 2.8f, 19f, 2.8f, 19f, 21.2f, 16.5f, 19.4f, 14f, 21.2f,
                11.5f, 19.4f, 9f, 21.2f, 6.5f, 19.4f, 5f, 21.2f,
            )
            ln(c, w, u, 8.4f, 8f, 15.6f, 8f)
            ln(c, w, u, 8.4f, 12.2f, 15.6f, 12.2f)
        }
        AppIcon.Document -> {
            poly(c, w, u, true, 5.6f, 2.8f, 14f, 2.8f, 19f, 7.8f, 19f, 21.2f, 5.6f, 21.2f)
            poly(c, w, u, false, 14f, 2.8f, 14f, 7.8f, 19f, 7.8f)
            ln(c, w, u, 8.6f, 13f, 15.6f, 13f)
            ln(c, w, u, 8.6f, 16.8f, 13.4f, 16.8f)
        }
        AppIcon.Folder -> poly(
            c, w, u, true,
            3f, 19.4f, 3f, 5.4f, 9.4f, 5.4f, 11.6f, 8.4f, 21f, 8.4f, 21f, 19.4f,
        )
        AppIcon.Cloud -> {
            arc(c, w, u, 6f, 5f, 19.6f, 18.6f, 190f, 165f)
            arc(c, w, u, 2.4f, 11f, 11.2f, 19.8f, 90f, 145f)
            ln(c, w, u, 6.8f, 19.4f, 17.6f, 19.4f)
            arc(c, w, u, 14.6f, 10.8f, 22.6f, 18.8f, 300f, 130f)
        }
        AppIcon.Upload -> {
            poly(c, w, u, false, 4f, 15.4f, 4f, 20.4f, 20f, 20.4f, 20f, 15.4f)
            ln(c, w, u, 12f, 3.4f, 12f, 15f)
            poly(c, w, u, false, 6.8f, 8.6f, 12f, 3.4f, 17.2f, 8.6f)
        }
        AppIcon.Download -> {
            poly(c, w, u, false, 4f, 15.4f, 4f, 20.4f, 20f, 20.4f, 20f, 15.4f)
            ln(c, w, u, 12f, 3.4f, 12f, 15f)
            poly(c, w, u, false, 6.8f, 9.8f, 12f, 15f, 17.2f, 9.8f)
        }
        AppIcon.Refresh -> {
            arc(c, w, u, 3.6f, 3.6f, 20.4f, 20.4f, 60f, 250f)
            poly(c, w, u, false, 16.2f, 3.2f, 20.2f, 6.4f, 16.4f, 9.6f)
        }
        AppIcon.Check -> poly(c, w, u, false, 4.4f, 12.6f, 9.6f, 17.8f, 19.6f, 6.6f)
        AppIcon.Close -> {
            ln(c, w, u, 6f, 6f, 18f, 18f)
            ln(c, w, u, 18f, 6f, 6f, 18f)
        }
        AppIcon.Plus -> {
            ln(c, w, u, 12f, 5f, 12f, 19f)
            ln(c, w, u, 5f, 12f, 19f, 12f)
        }
        AppIcon.Mail -> {
            box(c, w, u, 2.8f, 5f, 21.2f, 19f, 2.6f)
            poly(c, w, u, false, 3.4f, 6.4f, 12f, 13.2f, 20.6f, 6.4f)
        }
        AppIcon.Phone -> poly(
            c, w, u, true,
            7.6f, 2.8f, 10.6f, 2.8f, 12.2f, 7.6f, 9.8f, 9.6f,
            11.2f, 12.8f, 14.4f, 14.2f, 16.4f, 11.8f, 21.2f, 13.4f,
            21.2f, 16.4f, 18.6f, 20.6f, 12.4f, 19f, 5f, 11.6f, 3.4f, 5.4f,
        )
        AppIcon.Globe -> {
            circ(c, w, u, 12f, 12f, 9f)
            ln(c, w, u, 3f, 12f, 21f, 12f)
            arc(c, w, u, 7.4f, 3f, 16.6f, 21f, 90f, 180f)
            arc(c, w, u, 7.4f, 3f, 16.6f, 21f, 270f, 180f)
        }
        AppIcon.Clock -> {
            circ(c, w, u, 12f, 12f, 9f)
            poly(c, w, u, false, 12f, 6.6f, 12f, 12.4f, 16.2f, 14.6f)
        }
        AppIcon.Warning -> {
            poly(c, w, u, true, 12f, 3f, 22f, 20.4f, 2f, 20.4f)
            ln(c, w, u, 12f, 9.4f, 12f, 14.6f)
            dot(c, u, 12f, 17.6f, 1.2f)
        }
        AppIcon.Star -> poly(
            c, w, u, true,
            12f, 2.8f, 14.9f, 9.1f, 21.6f, 9.9f, 16.6f, 14.4f,
            18f, 21.2f, 12f, 17.8f, 6f, 21.2f, 7.4f, 14.4f, 2.4f, 9.9f, 9.1f, 9.1f,
        )
        AppIcon.Send -> {
            poly(c, w, u, true, 21f, 3f, 2.6f, 10.4f, 10.2f, 13.8f, 13.6f, 21.4f)
            ln(c, w, u, 21f, 3f, 10.2f, 13.8f)
        }
        AppIcon.Home -> {
            poly(c, w, u, false, 3f, 11.2f, 12f, 3.2f, 21f, 11.2f)
            poly(c, w, u, false, 5.4f, 9.6f, 5.4f, 20.6f, 18.6f, 20.6f, 18.6f, 9.6f)
            ln(c, w, u, 10f, 20.6f, 10f, 14.4f)
            ln(c, w, u, 14f, 20.6f, 14f, 14.4f)
        }
        AppIcon.Chart -> {
            ln(c, w, u, 3.4f, 20.6f, 20.6f, 20.6f)
            ln(c, w, u, 7f, 20.6f, 7f, 12.6f)
            ln(c, w, u, 12f, 20.6f, 12f, 6.6f)
            ln(c, w, u, 17f, 20.6f, 17f, 15.6f)
        }
        AppIcon.Eye -> {
            val p = Path().apply {
                moveTo(2.4f * u, 12f * u)
                cubicTo(6f * u, 5.6f * u, 18f * u, 5.6f * u, 21.6f * u, 12f * u)
                cubicTo(18f * u, 18.4f * u, 6f * u, 18.4f * u, 2.4f * u, 12f * u)
                close()
            }
            drawPath(p, c, style = Stroke(w, join = StrokeJoin.Round))
            circ(c, w, u, 12f, 12f, 3.2f)
        }
        AppIcon.Terminal -> {
            box(c, w, u, 2.6f, 4.4f, 21.4f, 19.6f, 2.6f)
            poly(c, w, u, false, 6.6f, 9.4f, 10.2f, 12.4f, 6.6f, 15.4f)
            ln(c, w, u, 12.6f, 15.6f, 17.4f, 15.6f)
        }
        AppIcon.Logout -> {
            poly(c, w, u, false, 14f, 3.4f, 4f, 3.4f, 4f, 20.6f, 14f, 20.6f)
            ln(c, w, u, 9.6f, 12f, 21f, 12f)
            poly(c, w, u, false, 17.2f, 8f, 21.2f, 12f, 17.2f, 16f)
        }
        AppIcon.Pin -> {
            poly(
                c, w, u, true,
                12f, 2.6f, 18.6f, 8.2f, 18.6f, 13.4f, 12f, 21.4f, 5.4f, 13.4f, 5.4f, 8.2f,
            )
            circ(c, w, u, 12f, 9.8f, 2.8f)
        }
        AppIcon.Sparkle -> {
            poly(
                c, w, u, true,
                12f, 2.6f, 14.2f, 9.8f, 21.4f, 12f, 14.2f, 14.2f,
                12f, 21.4f, 9.8f, 14.2f, 2.6f, 12f, 9.8f, 9.8f,
            )
        }
    }
}
