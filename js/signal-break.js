const signalBreak = document.querySelector('.signal-break');
const signalCode = document.querySelector('.signal-break__code');
const reducedMotion = window.matchMedia('(prefers-reduced-motion: reduce)');

const initialDelay = 13000;
const restingDuration = 40000;
const charactersPerSecond = 640;
const completionDelay = 700;

let sourceText = '';
let activationTimer;
let typingFrame;
let completionTimer;

async function getSignalbreakText() {
  const embeddedText = globalThis.__LANDING_DATA__?.signalbreak;
  if (typeof embeddedText === 'string' && embeddedText.trim()) return embeddedText;

  try {
    const response = await fetch('data/signalbreak.c');
    if (!response.ok) throw new Error(`Signalbreak data request failed (${response.status})`);
    return await response.text();
  } catch (error) {
    console.error('Unable to load Signalbreak text:', error);
    return '// AURORA CONTROL SIMULATION\nSTATE reactor := SAFE_HOLD;\nINTERLOCK all_channels := VERIFIED;\n';
  }
}

function scheduleActivation(delay) {
  window.clearTimeout(activationTimer);
  activationTimer = window.setTimeout(startTyping, delay);
}

function finishTyping() {
  window.cancelAnimationFrame(typingFrame);
  window.clearTimeout(completionTimer);
  document.body.classList.remove('signal-break--active');
  signalBreak.classList.remove('is-active');
  scheduleActivation(restingDuration);
}

function startTyping() {
  if (reducedMotion.matches || document.hidden || !sourceText) {
    scheduleActivation(restingDuration);
    return;
  }

  window.clearTimeout(completionTimer);
  signalCode.textContent = '';
  signalBreak.scrollTop = 0;
  signalBreak.classList.add('is-active');
  document.body.classList.add('signal-break--active');

  const startedAt = performance.now();
  let output = '';
  let renderedCharacters = 0;

  function typeFrame(timestamp) {
    const elapsed = timestamp - startedAt;
    const targetCharacters = Math.min(
      sourceText.length,
      Math.floor((elapsed / 1000) * charactersPerSecond),
    );
    const newCharacters = targetCharacters - renderedCharacters;

    if (newCharacters > 0) {
      output += sourceText.slice(renderedCharacters, targetCharacters);
      renderedCharacters = targetCharacters;
      signalCode.textContent = output;
      signalBreak.scrollTop = signalBreak.scrollHeight;
    }

    if (renderedCharacters < sourceText.length) {
      typingFrame = window.requestAnimationFrame(typeFrame);
    } else {
      completionTimer = window.setTimeout(finishTyping, completionDelay);
    }
  }

  typingFrame = window.requestAnimationFrame(typeFrame);
}

async function initializeSignalBreak() {
  if (!signalBreak || !signalCode) return;

  sourceText = await getSignalbreakText();
  if (!reducedMotion.matches) scheduleActivation(initialDelay);
}

reducedMotion.addEventListener('change', ({ matches }) => {
  window.clearTimeout(activationTimer);
  window.clearTimeout(completionTimer);
  window.cancelAnimationFrame(typingFrame);
  document.body.classList.remove('signal-break--active');
  signalBreak?.classList.remove('is-active');

  if (!matches && sourceText) scheduleActivation(initialDelay);
});

initializeSignalBreak();
