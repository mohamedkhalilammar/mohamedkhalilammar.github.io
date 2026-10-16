import { chromium } from "playwright";
import assert from "node:assert/strict";
import fs from "node:fs";

const base = process.env.FRONTEND_URL || "http://localhost:3001";
const output = "/tmp/portfolio-review";
fs.mkdirSync(output, { recursive: true });
const browser = await chromium.launch({ channel: "chromium", headless: true, args: ["--disable-gpu"] });
const context = await browser.newContext({ reducedMotion: "reduce" });
const page = await context.newPage();
const errors = [];
page.on("pageerror", error => { errors.push(error.message); console.error(error.message); });
const routes = ["/", "/blog", "/writeups/mojo-warmup", "/project/haniny-ai-driver-assistant", "/arcade"];
try {
  for (const theme of (process.argv.includes("--interactions-only") ? [] : ["midnight", "amber-light"])) {
    await context.addInitScript(value => localStorage.setItem("portfolio-theme", value), theme);
    for (const width of [1440, 1024, 768, 390]) {
      await page.setViewportSize({ width, height: 900 });
      for (const route of routes) {
        const response = await page.goto(base + route);
        assert.equal(response.status(), 200, route);
        await page.locator("main h1").first().waitFor();
        await page.evaluate(() => document.fonts.ready);
        const overflow = await page.evaluate(() => ({
          width: document.documentElement.clientWidth,
          scroll: document.documentElement.scrollWidth,
          main: document.querySelector("main").getBoundingClientRect().right,
        }));
        assert.ok(overflow.scroll <= width + 1 && overflow.main <= width + 1, route + " overflows at " + width + ": " + JSON.stringify(overflow));
        if (width === 390 || width === 1440) {
          await page.screenshot({ path: output + "/" + theme + "-" + width + "-" + (route.replaceAll("/", "_") || "home") + ".png", animations: "disabled" });
        }
      }
      console.log("Responsive routes passed:", theme, width);
    }
  }
  await page.setViewportSize({ width: 390, height: 900 });
  await page.goto(base + "/blog");
  await page.getByRole("searchbox").fill("warmup");
  await page.waitForTimeout(300);
  assert.equal(await page.locator(".journal-row").count(), 1);
  assert.ok(page.url().includes("q=warmup"));
  await page.reload();
  await page.getByRole("searchbox").waitFor();
  await page.waitForTimeout(300);
  assert.equal(await page.getByRole("searchbox").inputValue(), "warmup");
  await page.getByRole("searchbox").fill("nothing-matches-this-query");
  await page.getByRole("heading", { name: "No articles match these filters." }).waitFor();
  await page.getByRole("button", { name: "Show all articles" }).click();
  assert.equal(await page.locator(".journal-row").count(), 6);
  await page.getByRole("button", { name: /Engineering/ }).click();
  assert.equal(await page.locator(".journal-row").count(), 1);
  await page.getByRole("button", { name: /All writing/ }).click();
  await page.getByLabel("Topic", { exact: true }).selectOption("Python");
  await page.waitForFunction(() => document.querySelectorAll(".journal-row").length === 4);
  assert.equal(await page.locator(".journal-row").count(), 4);
  await page.getByRole("button", { name: "Menu" }).click();
  await page.getByRole("dialog").waitFor();
  await page.keyboard.press("Escape");
  assert.equal(await page.getByRole("dialog").count(), 0);
  await page.goto(base + "/writeups/mojo-warmup");
  assert.equal(await page.locator("main h1").count(), 1);
  await page.getByText("Recovered flag", { exact: false }).last().click();
  assert.equal(await page.locator(".article-flag").getAttribute("open"), "");
  await page.goto(base + "/project/haniny-ai-driver-assistant");
  await page.locator(".project-gallery button").first().click();
  await page.getByRole("dialog").waitFor();
  await page.keyboard.press("ArrowRight");
  assert.ok((await page.locator(".gallery-dialog-bar").innerText()).includes("Image 2"));
  await page.keyboard.press("Escape");
  assert.equal(await page.getByRole("dialog").count(), 0);
  // Mock delivery: never send real messages while checking the form.
  await page.route("https://formspree.io/**", route => route.fulfill({ status: 500, body: "{}" }));
  await page.goto(base + "/#contact");
  await page.getByRole("button", { name: "Menu", exact: true }).click();
  await page.getByRole("dialog").waitFor();
  await page.keyboard.press("Escape");
  await page.getByLabel("Email address").fill("test@example.com");
  await page.getByLabel("Message", { exact: true }).fill("Local interface verification.");
  await page.getByRole("button", { name: "Send Message" }).click();
  await page.locator(".contact-form-ui [role='alert']").waitFor();
  await page.unroute("https://formspree.io/**");
  await page.route("https://formspree.io/**", route => route.fulfill({ status: 200, contentType: "application/json", body: "{}" }));
  await page.getByRole("button", { name: "Send Message" }).click();
  await page.getByRole("button", { name: "Send another message" }).waitFor();
  assert.deepEqual(errors, []);
  console.log("Search, filters, URL restoration, menu, spoilers, gallery and mocked contact states passed.");
  console.log("Screenshots:", output);
} finally {
  await browser.close();
}
