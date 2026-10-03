#pragma once
#include <Arduino.h>

// Pagina web servita dall'ESP32 (http://192.168.4.1)
const char INDEX_HTML[] PROGMEM = R"HTML(<!doctype html>
<html lang="it"><head><meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>SOG-Drop</title>
<style>
:root{--bg:#f3f6f2;--card:#fff;--fg:#1d2a1f;--mut:#667066;--acc:#2e7d32;--err:#c62828;--bd:#d6ddd4}
@media (prefers-color-scheme:dark){:root{--bg:#121813;--card:#1c241e;--fg:#e3ebe4;--mut:#98a59a;--bd:#33403a;--acc:#66bb6a;--err:#ef5350}}
*{box-sizing:border-box}body{margin:0;font:15px system-ui,sans-serif;background:var(--bg);color:var(--fg)}
header{background:var(--acc);color:#fff;padding:12px 16px;font-weight:600;font-size:18px}
nav{display:flex;background:var(--card);border-bottom:1px solid var(--bd);position:sticky;top:0;z-index:1}
nav button{flex:1;border:0;background:none;padding:12px 4px;color:var(--fg);font-size:15px;border-bottom:3px solid transparent}
nav button.on{border-color:var(--acc);font-weight:600}
main{padding:12px 16px;max-width:900px;margin:auto}
section{display:none}section.on{display:block}
.card{background:var(--card);border:1px solid var(--bd);border-radius:10px;padding:12px;margin-bottom:12px}
h3{margin:0 0 8px;font-size:16px}
.row{display:flex;gap:8px;align-items:center;flex-wrap:wrap;margin:6px 0}
.big{font-size:22px;font-weight:600}.mut{color:var(--mut);font-size:13px}
.b{border:0;border-radius:8px;padding:10px 14px;background:var(--acc);color:#fff;font-size:15px}
.b2{background:var(--bd);color:var(--fg)}.stop{background:var(--err);color:#fff}
.sm{padding:6px 10px}
input,select{font-size:15px;padding:6px;border:1px solid var(--bd);border-radius:6px;background:var(--bg);color:var(--fg)}
input[type=number]{width:72px}
.al{padding:6px 8px;border-radius:6px;margin:4px 0;font-size:13px;background:#e6950026}.al.e{background:#c6282826}
.tw{overflow-x:auto}table{border-collapse:collapse;font-size:13px}
td,th{padding:4px;border-bottom:1px solid var(--bd);text-align:center;white-space:nowrap}
td input[type=number]{width:58px}
pre{white-space:pre-wrap;font-size:12px;max-height:70vh;overflow:auto;margin:0}
.ok{color:var(--acc)}.ko{color:var(--err);font-weight:600}
</style></head><body>
<header>SOG-Drop</header>
<nav><button data-t="st" class="on">Stato</button><button data-t="pl">Piano</button><button data-t="ma">Manuale</button><button data-t="lg">Registro</button></nav>
<main>
<section id="st" class="on">
 <div class="card"><h3>Orologio</h3><div class="big" id="time">--</div><div id="clockWarn" class="ko"></div>
  <div class="row"><button class="b b2" onclick="syncTime()">Sincronizza ora dal telefono</button></div></div>
 <div class="card"><h3>Programma</h3><div id="sched"></div>
  <div class="row"><button class="b" onclick="runNow()">Irriga ora</button><button class="b b2" id="skipBtn" onclick="skip()">Salta prossima</button><button class="b stop" onclick="stopAll()">STOP</button></div></div>
 <div class="card"><h3>Impianto</h3><div id="plant"></div></div>
 <div class="card"><h3>Avvisi</h3><div id="alerts"></div>
  <div class="row"><button class="b b2 sm" onclick="post('/api/alerts/clear').then(load)">Cancella avvisi</button></div></div>
</section>

<section id="pl">
 <div class="card"><h3>Piano di coltivazione</h3>
  <div class="row"><label>Nome <input id="pName" maxlength="30"></label><label><input type="checkbox" id="pActive"> Attivo</label></div>
  <div class="row"><label>Inizio <input type="date" id="pStart"></label><label>Ora irrigazione <input type="time" id="pTime"></label></div>
  <div class="row"><label>Fertilizzanti <select id="pNumF"><option>1</option><option>2</option><option>3</option><option>4</option><option>5</option></select></label></div>
  <div class="row" id="fNames"></div>
  <div class="row" id="plantsOn"></div>
 </div>
 <div class="card"><h3>Settimane</h3>
  <div class="mut">Litri per pianta nell'intera settimana, divisi tra i giorni spuntati. Fertilizzanti in ml per litro d'acqua.</div>
  <div class="tw"><table id="weeks"></table></div>
  <div class="row"><button class="b b2 sm" onclick="addWeek()">+ Aggiungi settimana</button></div></div>
 <div class="card"><h3>Riepilogo</h3><div id="summary"></div></div>
 <div class="row"><button class="b" onclick="savePlan()">Salva piano</button><span id="planMsg"></span></div>
</section>

<section id="ma">
 <div class="card"><h3>Comandi manuali</h3><div class="mut">Disponibili solo quando non c'è un'irrigazione in corso.</div><div id="outs"></div>
  <div class="row"><button class="b stop" onclick="stopAll()">Spegni tutto</button></div></div>
 <div class="card"><h3>Bilancia</h3><div id="scaleInfo"></div>
  <div class="row"><button class="b b2 sm" onclick="tare()">1. Tara (vasca vuota)</button></div>
  <div class="row"><input type="number" id="calG" value="1000"> g <button class="b b2 sm" onclick="calScale()">2. Calibra con peso noto</button></div>
  <div class="mut">Svuota la vasca e premi Tara. Poi versa una quantità d'acqua pesata con una bilancia da cucina (es. 1000 g) e premi Calibra.</div></div>
 <div class="card"><h3>Calibrazione pompe fertilizzanti</h3>
  <div class="mut">Riempi prima il tubo. Metti l'uscita in un misurino, premi "Avvia 30 s", scrivi i ml raccolti e premi Salva.</div>
  <div id="fcal"></div>
  <div class="row"><label>Tempo di miscelazione <input type="number" id="mixS" min="0" max="120"> s</label><button class="b b2 sm" onclick="saveMix()">Salva</button></div></div>
</section>

<section id="lg"><div class="card">
 <div class="row"><button class="b b2 sm" onclick="loadLog()">Aggiorna</button><button class="b b2 sm" onclick="loadLog(1)">Registro precedente</button></div>
 <pre id="log"></pre></div></section>
</main>
<script>
const $=id=>document.getElementById(id);
const DAYS=['L','M','M','G','V','S','D'];
let S={},P=null,C=null,built=false;
document.querySelectorAll('nav button').forEach(b=>b.onclick=()=>{
 document.querySelectorAll('nav button,section').forEach(e=>e.classList.remove('on'));
 b.classList.add('on');$(b.dataset.t).classList.add('on');if(b.dataset.t=='lg')loadLog()});
async function api(u,o){const r=await fetch(u,o);const j=await r.json().catch(()=>({}));if(!r.ok)throw new Error(j.error||r.status);return j}
const post=(u,b)=>api(u,{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify(b||{})});
const fmt=(n,d=2)=>(+n).toFixed(d).replace('.',',');
const esc=s=>String(s).replace(/[&<>"]/g,c=>({'&':'&amp;','<':'&lt;','>':'&gt;','"':'&quot;'}[c]));
const popc=x=>{let c=0;for(;x;x>>=1)c+=x&1;return c};
const fail=e=>alert(e.message);

async function load(){
 try{S=await api('/api/status')}catch(e){$('time').textContent='Non connesso';return}
 $('time').textContent=S.clockOk?S.time:'Ora non impostata';
 $('clockWarn').textContent=S.clockOk?'':'Imposta l\'ora: senza ora corretta le irrigazioni programmate non partono.';
 const s=S.schedule;let h='';
 if(!s.planActive)h='<div class="ko">Piano non attivo</div>';
 else{h=`<div><b>${esc(s.planName)}</b></div>`;
  if(s.week>0)h+=`<div>Settimana ${s.week} di ${s.numWeeks}</div>`;
  else if(s.week==0)h+='<div>Il piano non è ancora iniziato</div>';
  else if(s.week<0)h+='<div>Piano terminato</div>';
  h+=s.next?`<div>Prossima irrigazione: <b>${s.next}</b> · ${fmt(s.nextL)} L per pianta</div>`:'<div class="mut">Nessuna irrigazione in programma</div>'}
 if(S.skipNext)h+='<div class="ko">La prossima irrigazione verrà saltata</div>';
 $('sched').innerHTML=h;$('skipBtn').textContent=S.skipNext?'Annulla salto':'Salta prossima';
 let p=`<div>Fase: <b>${S.phase}</b></div>
 <div>Vasca: <b>${S.scaleCal?fmt(S.grams/1000)+' L':'bilancia non calibrata'}</b> ${S.scaleOk?'':'<span class="ko">bilancia non risponde</span>'}</div>
 <div>Galleggiante: ${S.tankFull?'<span class="ko">LIVELLO MASSIMO</span>':'<span class="ok">ok</span>'}</div>`;
 if(S.cycle)p+=`<div>Erogato: ${S.cycle.delivered.map((v,i)=>`P${i+1} ${fmt(v)}/${fmt(S.cycle.perPlantL)} L`).join(' · ')}</div>`;
 const on=S.outNames.filter((n,i)=>S.outputs[i]);
 p+=`<div class="mut">Uscite attive: ${on.length?on.join(', '):'nessuna'}</div>`;
 $('plant').innerHTML=p;
 $('alerts').innerHTML=S.alerts.length?S.alerts.map(a=>`<div class="al ${a.e?'e':''}">${esc(a.t)}: ${esc(a.m)}</div>`).join(''):'<div class="mut">Nessun avviso</div>';
 if(!built){built=true;$('outs').innerHTML=S.outNames.map((n,i)=>`<div class="row"><span style="flex:1" id="on${i}">${esc(n)}</span><input type="number" id="os${i}" value="5" min="1" max="600"> s <button class="b b2 sm" onclick="manual(${i})">Avvia</button></div>`).join('')}
 S.outputs.forEach((v,i)=>{$('on'+i).className=v?'ok':''});
 $('scaleInfo').textContent=`Lettura grezza: ${S.raw} · Peso: ${S.scaleCal?fmt(S.grams,0)+' g':'non calibrata'}`;
}
function syncTime(){post('/api/time',{epoch:Math.floor(Date.now()/1000)}).then(load).catch(fail)}
function runNow(){const l=prompt('Litri per pianta (vuoto = dose della settimana in corso)','');if(l===null)return;
 post('/api/run',l?{litres:parseFloat(l.replace(',','.'))}:{}).then(load).catch(fail)}
function skip(){post('/api/skip',{skip:!S.skipNext}).then(load).catch(fail)}
function stopAll(){post('/api/stop').then(load).catch(fail)}
function manual(i){post('/api/manual',{out:i,sec:+$('os'+i).value}).then(load).catch(fail)}

async function loadPlan(){P=await api('/api/plan');renderPlan()}
function perIrr(w){const n=popc(w.d);return n&&w.l>0?fmt(w.l/n):'—'}
function renderPlan(){
 $('pName').value=P.name;$('pActive').checked=P.active;$('pStart').value=P.start;$('pTime').value=P.time;$('pNumF').value=P.numFert;
 let h='';for(let i=0;i<P.numFert;i++)h+=`<label>F${i+1} <input data-fn="${i}" value="${esc(P.fertNames[i])}" maxlength="15" style="width:110px"></label>`;
 $('fNames').innerHTML=h;
 $('plantsOn').innerHTML='Piante attive: '+P.plants.map((v,i)=>`<label><input type="checkbox" data-po="${i}" ${v?'checked':''}> ${i+1}</label>`).join(' ');
 const F=P.fertNames.slice(0,P.numFert);
 let t='<tr><th>Sett.</th><th>L/pianta</th>'+DAYS.map(d=>`<th>${d}</th>`).join('')+F.map(n=>`<th>${esc(n)}<br>ml/L</th>`).join('')+'<th>L per<br>irrig.</th><th></th></tr>';
 P.weeks.forEach((w,i)=>{t+=`<tr><td>${i+1}</td><td><input type="number" step="0.1" min="0" data-w="${i}" data-k="l" value="${w.l}"></td>`
  +DAYS.map((d,j)=>`<td><input type="checkbox" data-w="${i}" data-d="${j}" ${w.d>>j&1?'checked':''}></td>`).join('')
  +F.map((n,j)=>`<td><input type="number" step="0.1" min="0" data-w="${i}" data-f="${j}" value="${w.f[j]}"></td>`).join('')
  +`<td>${perIrr(w)}</td><td><button class="b b2 sm" onclick="dupWeek(${i})" title="Duplica">⧉</button> <button class="b b2 sm" onclick="delWeek(${i})" title="Elimina">✕</button></td></tr>`});
 $('weeks').innerHTML=t;summary();
}
function readPlan(){
 P.name=$('pName').value;P.active=$('pActive').checked;P.start=$('pStart').value;P.time=$('pTime').value;P.numFert=+$('pNumF').value;
 document.querySelectorAll('[data-fn]').forEach(e=>P.fertNames[+e.dataset.fn]=e.value);
 document.querySelectorAll('[data-po]').forEach(e=>P.plants[+e.dataset.po]=e.checked);
 document.querySelectorAll('[data-w]').forEach(e=>{const w=P.weeks[+e.dataset.w];
  if(e.dataset.k)w.l=parseFloat(e.value)||0;
  else if(e.dataset.d!==undefined){const b=1<<+e.dataset.d;w.d=e.checked?w.d|b:w.d&~b}
  else w.f[+e.dataset.f]=parseFloat(e.value)||0});
}
$('pl').addEventListener('change',()=>{readPlan();renderPlan()});
const copy=o=>JSON.parse(JSON.stringify(o));
function addWeek(){readPlan();if(P.weeks.length>=30)return;P.weeks.push(copy(P.weeks[P.weeks.length-1]));renderPlan()}
function dupWeek(i){readPlan();if(P.weeks.length>=30)return;P.weeks.splice(i+1,0,copy(P.weeks[i]));renderPlan()}
function delWeek(i){readPlan();if(P.weeks.length>1&&confirm('Eliminare la settimana '+(i+1)+'?')){P.weeks.splice(i,1);renderPlan()}}
function summary(){
 const np=P.plants.filter(x=>x).length,nf=P.numFert,s=S.schedule||{};
 const cur=s.week>0?s.week:(s.week<0?999:1);
 let tot=0,all=Array(nf).fill(0),rem=Array(nf).fill(0);
 P.weeks.forEach((w,i)=>{const L=w.l*np;tot+=L;for(let j=0;j<nf;j++){const m=w.f[j]*L;all[j]+=m;if(i+1>=cur)rem[j]+=m}});
 let h=`<div>Acqua totale del piano: <b>${fmt(tot,1)} L</b> (${np} piante)</div><table><tr><th></th><th>Intero piano</th><th>Da questa settimana</th></tr>`;
 for(let j=0;j<nf;j++)h+=`<tr><td>${esc(P.fertNames[j])}</td><td>${fmt(all[j],0)} ml</td><td>${fmt(rem[j],0)} ml</td></tr>`;
 h+='</table><div class="mut">Prima di partire, lascia nelle taniche almeno questa quantità di concentrato più una scorta: le pompe non devono pescare a vuoto.</div>';
 $('summary').innerHTML=h;
}
function savePlan(){readPlan();post('/api/plan',P).then(()=>{$('planMsg').textContent='Salvato';load()}).catch(e=>$('planMsg').textContent='Errore: '+e.message)}

async function loadCalib(){C=await api('/api/calib');$('mixS').value=C.mixSeconds;
 $('fcal').innerHTML=C.fertMlPerSec.map((r,i)=>`<div class="row"><span style="flex:1">${esc(P?P.fertNames[i]:'F'+(i+1))}: ${r>0?fmt(r,2)+' ml/s':'<span class="ko">non calibrata</span>'}</span>
 <button class="b b2 sm" onclick="fRun(${i})">Avvia 30 s</button><input type="number" id="fml${i}" placeholder="ml" step="0.1"><button class="b b2 sm" onclick="fSave(${i})">Salva</button></div>`).join('')}
function fRun(i){post('/api/manual',{out:S.fert0+i,sec:30}).then(load).catch(fail)}
function fSave(i){const ml=parseFloat($('fml'+i).value);if(!(ml>0))return alert('Inserisci i ml raccolti');C.fertMlPerSec[i]=ml/30;post('/api/calib',C).then(loadCalib).catch(fail)}
function saveMix(){C.mixSeconds=+$('mixS').value;post('/api/calib',C).then(loadCalib).catch(fail)}
function tare(){if(confirm('La vasca è completamente vuota?'))post('/api/tare').then(load).catch(fail)}
function calScale(){post('/api/scalecal',{grams:+$('calG').value}).then(load).catch(fail)}
async function loadLog(old){const r=await fetch('/api/log'+(old?'?old=1':''));const t=await r.text();$('log').textContent=t.split('\n').reverse().join('\n')}

load().finally(async()=>{await loadPlan().catch(()=>{});loadCalib().catch(()=>{})});
setInterval(load,2000);
</script></body></html>)HTML";
