/* Optional browser checks. Set PLAYWRIGHT_MODULE to an installed Playwright module.
   The application itself has no npm dependencies. Start `make gui` first.
   PLAYWRIGHT_MODULE=/path/to/playwright node tests/gui_browser.cjs
*/
const {chromium}=require(process.env.PLAYWRIGHT_MODULE || 'playwright');
const assert=require('node:assert/strict');
const fs=require('node:fs/promises');
const path=require('node:path');
const ROOT=path.resolve(__dirname,'..');
const OUTPUT=path.join(ROOT,'results/test-gui');
const URL=process.env.GUI_URL || 'http://127.0.0.1:8765';
let checks=0;
function check(value,message){assert.ok(value,message);checks++;}

(async()=>{
    await fs.mkdir(OUTPUT,{recursive:true});
    const launchOptions={headless:true};
    if(process.env.CHROME_PATH)launchOptions.executablePath=process.env.CHROME_PATH;
    const browser=await chromium.launch(launchOptions);
    const context=await browser.newContext({viewport:{width:1512,height:1100},reducedMotion:'reduce',acceptDownloads:true});
    const page=await context.newPage();
    page.setDefaultTimeout(15000);
    const errors=[];
    page.on('pageerror',error=>errors.push(error.message));
    page.on('console',message=>{if(message.type()==='error'&&!message.text().includes('status of 400'))errors.push(message.text());});
    async function text(selector,value){await page.locator(selector).filter({hasText:value}).waitFor();checks++;}
    async function finish(){await page.locator('body.loading').waitFor({state:'hidden'});}
    async function dataDialog(){await page.locator('#change-data').click();await page.locator('#data-dialog').waitFor({state:'visible'});}
    async function sample(){await dataDialog();await page.locator('#load-sample').click();await finish();await text('#stat-campaign','472');}
    try{
        await page.goto(URL);
        await text('#stat-campaign','472');
        await finish();
        check(await page.locator('#stat-flow').innerText()==='36','normal max flow');
        check(await page.locator('#network-graph .node').count()===12,'real graph nodes');
        check(!await page.evaluate(()=>document.documentElement.scrollWidth>innerWidth),'desktop fits');
        await page.locator('#network-graph [data-user="11"]').click();
        await text('#user-detail','0 → 5 → 10 → 11');
        check(await page.locator('#network-graph .edge.highlight').count()===3,'highlighted route edges');
        const oldTransform=await page.locator('#graph-layer').getAttribute('transform');
        await page.locator('#zoom-in').click();
        check(await page.locator('#graph-layer').getAttribute('transform')!==oldTransform,'zoom works');
        await page.locator('#zoom-reset').click();
        await page.locator('#graph-search').fill('user_11');
        check(await page.locator('#network-graph .node.dim').count()===11,'graph search filters');
        await page.locator('#graph-search').fill('');
        await page.screenshot({path:path.join(OUTPUT,'overview-desktop.png'),fullPage:true});

        await page.locator('[data-scenario="shock"]').click();
        await text('#stat-campaign','265');await finish();
        check(await page.locator('#stat-flow').innerText()==='8','shock max flow');
        check(await page.locator('#stat-communities').innerText()==='2','shock communities');
        check(await page.locator('#budget').inputValue()==='10','shock budget applied');
        await page.locator('[data-view="communities"]').click();
        check(await page.locator('.community-card').count()===2,'community directory');
        await page.locator('#community-search').fill('user_2');
        check(await page.locator('.community-card').count()===1,'community member search');
        await page.locator('[data-community-user="2"]').click();
        await text('#user-detail','user_2');
        check(await page.locator('#view-overview').isVisible(),'member opens overview');
        await page.locator('[data-view="campaign"]').click();
        check(await page.locator('#candidate-table tr').count()===12,'candidate table');
        await text('#plan-summary','265');
        check(await page.locator('#topic-assignments .assignment').count()===3,'real creator-topic matching');
        await page.screenshot({path:path.join(OUTPUT,'campaign-desktop.png'),fullPage:true});

        await page.locator('#budget').fill('0');await page.locator('#run-analysis').click();
        await text('#stat-campaign','0');await finish();
        check(await page.locator('#candidate-table .decision-selected').count()===0,'zero-budget plan');
        await page.locator('#budget').fill('-1');await page.locator('#run-analysis').click();
        await text('#message','Enter an integer campaign budget');
        check(await page.locator('#stat-campaign').innerText()==='0','invalid input keeps previous result');
        await sample();

        await page.locator('[data-view="lab"]').click();
        for(const method of ['dijkstra','bellman','floyd']){
            await page.locator('#route-method').selectOption(method);
            await page.locator('#route-form button[type="submit"]').click();
            await text('#route-result','delay=6');
            await text('#route-result',method);
        }
        await page.locator('#route-target').fill('999');
        await page.locator('#route-form button[type="submit"]').click();
        await page.locator('#route-result.error').waitFor();checks++;
        await page.locator('#route-target').fill('11');
        await page.locator('#match-text').fill('aaaaa');await page.locator('#match-pattern').fill('aaa');
        for(const method of ['naive','rk','fa','kmp']){
            await page.locator('#match-method').selectOption(method);
            await page.locator('#match-form button[type="submit"]').click();
            await text('#match-result','Byte offsets: 0, 1, 2');
            await page.locator('#match-form button[type="submit"]').waitFor({state:'visible'});
            await page.waitForFunction(()=>!document.querySelector('#match-form button').disabled);
        }
        await page.locator('#run-workbench').click();
        await text('#workbench-output','CRT counter reconstruction=23');
        const benchmarkPromise=page.waitForEvent('download');
        await page.locator('#run-benchmark').click();
        const benchmark=await benchmarkPromise;
        await benchmark.saveAs(path.join(OUTPUT,'browser-benchmarks.csv'));
        check((await fs.readFile(path.join(OUTPUT,'browser-benchmarks.csv'),'utf8')).trim().split('\n').length===123,'downloaded benchmark rows');
        await text('#workbench-output','122 cases across 36 variants');
        await page.screenshot({path:path.join(OUTPUT,'lab-desktop.png'),fullPage:true});

        await page.locator('#export-open').click();
        const jsonPromise=page.waitForEvent('download');await page.locator('#export-json').click();
        const jsonDownload=await jsonPromise;await jsonDownload.saveAs(path.join(OUTPUT,'browser-report.json'));
        const exported=JSON.parse(await fs.readFile(path.join(OUTPUT,'browser-report.json'),'utf8'));
        check(exported.campaign_value===472 && exported.users===12,'engine JSON export');
        const csvPromise=page.waitForEvent('download');await page.locator('#export-csv').click();
        const csvDownload=await csvPromise;await csvDownload.saveAs(path.join(OUTPUT,'browser-report.csv'));
        check((await fs.readFile(path.join(OUTPUT,'browser-report.csv'),'utf8')).trim().split('\n').length===13,'engine CSV export');
        await page.locator('[data-close="export-dialog"]').click();

        await dataDialog();await page.locator('#dataset-path').fill('missing-directory');
        await page.locator('#load-path-form button').click();await text('#data-error','does not exist');
        await page.locator('#dataset-path').fill('data/sample');
        await page.locator('#load-path-form button').click();await finish();
        await text('#dataset-name','sample');
        await dataDialog();
        await page.locator('#csv-upload').setInputFiles(['users.csv','topics.csv','posts.csv','relationships.csv','interactions.csv'].map(name=>path.join(ROOT,'data/sample',name)));
        await page.locator('#data-dialog').waitFor({state:'hidden'});await finish();
        await text('#dataset-name','Imported CSV network');
        await text('#stat-campaign','472');

        await dataDialog();await page.locator('#generate-count').fill('10000');
        await page.locator('#generate-form button').click();
        await text('#stat-users','10,000');await finish();
        await text('#stat-campaign','1,233');
        check(await page.locator('#stat-flow').innerText()==='Not computed','flow limit explained');
        check(await page.locator('#network-graph .node').count()<=102,'bounded interactive graph');
        await page.locator('[data-view="overview"]').click();
        check(await page.locator('#graph-disclosure').isVisible(),'sampling disclosed');
        await page.locator('[data-view="overview"]').click();
        await page.locator('#all-rankings').click();
        check(await page.locator('#all-ranking-rows tr').count()===50,'paginated stress rankings');
        await page.locator('#ranking-search').fill('user_2807');
        check(await page.locator('#all-ranking-rows tr').count()===1,'stress ranking search');
        await text('#all-ranking-rows','205');
        await page.locator('[data-close="rankings-dialog"]').click();
        await page.locator('[data-view="communities"]').click();
        check(await page.locator('.community-card').count()===9,'community pagination');
        await text('#community-pagination','1,401');
        await page.screenshot({path:path.join(OUTPUT,'communities-stress.png'),fullPage:true});
        await sample();await page.locator('[data-view="overview"]').click();

        await page.setViewportSize({width:390,height:844});
        check(!await page.evaluate(()=>document.documentElement.scrollWidth>innerWidth),'mobile overview fits');
        await page.screenshot({path:path.join(OUTPUT,'overview-mobile.png'),fullPage:true});
        await page.locator('.mobile-data-button').click();
        check(await page.locator('#data-dialog').isVisible(),'mobile dataset controls');
        await page.locator('[data-close="data-dialog"]').click();
        for(const view of ['communities','campaign','lab']){
            await page.locator(`[data-view="${view}"]`).click();
            check(!await page.evaluate(()=>document.documentElement.scrollWidth>innerWidth),`mobile ${view} fits`);
        }
        check(errors.length===0,`browser console errors: ${errors.join('; ')}`);
        console.log(`Passed ${checks} browser checks. Screenshots and downloads: results/test-gui/`);
    }finally{await browser.close();}
})().catch(error=>{console.error(error);process.exitCode=1;});
