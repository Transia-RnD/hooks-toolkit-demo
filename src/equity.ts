import {
  Wallet,
  Payment,
  TrustSet,
  xrpToDrops,
  OfferCreate,
  OfferCreateFlags,
} from '@transia/xrpl'
import { IssuedCurrencyAmount } from '@transia/xrpl/dist/npm/models/common'

import { Xrpld } from '@transia/hooks-toolkit'

// NOT EXPORTED
import {
  IC,
  XrplIntegrationTestContext,
  balance,
  close,
  serverUrl,
  setupClient,
  trust,
} from '@transia/hooks-toolkit/dist/npm/src/libs/xrpl-helpers'

const min = 0.99
const max = 1
const randomValue = (Math.random() * (max - min) + min).toFixed(2)

async function fundICWallet(
  testContext: XrplIntegrationTestContext,
  wallet: Wallet,
  ic: IC
) {
  const payTx: Payment = {
    TransactionType: 'Payment',
    Account: testContext.master.classicAddress,
    Destination: wallet.classicAddress,
    Amount: xrpToDrops(2000000),
  }
  await Xrpld.submit(testContext.client, {
    wallet: testContext.master,
    tx: payTx,
  })
  const amount: IssuedCurrencyAmount = {
    value: String(100000000),
    currency: ic.currency as string,
    issuer: ic.issuer as string,
  }
  const trustTx: TrustSet = {
    TransactionType: 'TrustSet',
    Account: wallet.classicAddress,
    LimitAmount: amount,
  }
  await Xrpld.submit(testContext.client, {
    wallet: wallet,
    tx: trustTx,
  })
}

async function sell(
  testContext: XrplIntegrationTestContext,
  wallet: Wallet,
  takerGets: IssuedCurrencyAmount,
  takerPays: string
) {
  const sellTx: OfferCreate = {
    TransactionType: 'OfferCreate',
    Account: wallet.classicAddress,
    TakerGets: takerGets,
    TakerPays: takerPays,
  }
  await Xrpld.submit(testContext.client, {
    wallet: wallet,
    tx: sellTx,
  })
}

async function buy(
  testContext: XrplIntegrationTestContext,
  wallet: Wallet,
  takerGets: string,
  takerPays: IssuedCurrencyAmount
) {
  const buyTx: OfferCreate = {
    TransactionType: 'OfferCreate',
    Account: wallet.classicAddress,
    TakerGets: takerGets,
    TakerPays: takerPays,
  }
  await Xrpld.submit(testContext.client, {
    wallet: wallet,
    tx: buyTx,
  })
}

async function init() {
  const testContext = (await setupClient(
    serverUrl
  )) as XrplIntegrationTestContext

  const aiWallet = Wallet.fromSeed('ssbyL7YhF7AURcLR2pFaC9CRvrp31')
  const applWallet = Wallet.fromSeed('shpxfmH1W3Fqt7kvsx3Dqni5neGfD')
  const aliceWallet = testContext.alice
  const bobWallet = testContext.bob
  // const carolWallet = testContext.carol

  const appl = IC.gw('APL', applWallet.classicAddress)

  // FUND APPL WALLET
  if ((await balance(testContext.client, applWallet.classicAddress)) < 2000) {
    const payTx: Payment = {
      TransactionType: 'Payment',
      Account: testContext.master.classicAddress,
      Destination: applWallet.classicAddress,
      Amount: xrpToDrops(2000),
    }
    await Xrpld.submit(testContext.client, {
      wallet: testContext.master,
      tx: payTx,
    })
  }

  // FUND AI WALLET
  await fundICWallet(testContext, aiWallet, appl)

  // FUND BUYER WALLET
  await fundICWallet(testContext, aliceWallet, appl)

  // FUND SELLER WALLET
  await fundICWallet(testContext, bobWallet, appl)

  await testContext.client.disconnect()
}

async function runCodeEvery10Seconds() {
  const testContext = (await setupClient(
    serverUrl
  )) as XrplIntegrationTestContext

  const aiWallet = Wallet.fromSeed('ssbyL7YhF7AURcLR2pFaC9CRvrp31')
  const applWallet = Wallet.fromSeed('shpxfmH1W3Fqt7kvsx3Dqni5neGfD')
  const aliceWallet = testContext.alice
  const bobWallet = testContext.bob

  const appl = IC.gw('APL', applWallet.classicAddress)

  let previousValue = parseFloat(randomValue)

  function walkValue() {
    // const minChange = -0.5
    // const maxChange = 0.5
    const minChange = -0.1
    const maxChange = 0.1
    const change = (
      Math.random() * (maxChange - minChange) +
      minChange
    ).toFixed(2)
    const newValue = (previousValue + parseFloat(change)).toFixed(2)
    previousValue = parseFloat(newValue)
    return newValue
  }

  const price = walkValue()
  const minVol = 10
  const maxVol = 100
  const volume = (Math.random() * (maxVol - minVol) + minVol).toFixed(2)
  console.log(`NEW PRICE: ${price}`)
  console.log(`NEW VOLUME: ${volume}`)

  const randomBuySell =
    parseFloat((Math.random() * (2 - 0) + 0).toFixed(2)) <= 1
  const randomWallet = parseInt(String(Math.random() * (3 - 0) + 0))
  const wallets = [applWallet, aliceWallet, bobWallet]
  const wallet = wallets[randomWallet]

  console.log(`ISSUER: ${appl.issuer}`)
  console.log(`CURRENCY: ${appl.currency}`)
  console.log(`RANDOM BUY/SELL: ${randomBuySell}`)

  // Randomly decide to buy or sell
  if (randomBuySell) {
    console.log(`BUYING with wallet: ${wallet.classicAddress}`)
    appl.set(parseFloat(volume))
    const takerPays: IssuedCurrencyAmount = {
      value: String(appl.value),
      currency: appl.currency as string,
      issuer: appl.issuer as string,
    }
    const takerGets: string = xrpToDrops(
      String(parseInt(String(parseFloat(price) * (appl.value as number))))
    )
    await buy(testContext, wallet, takerGets, takerPays)
  } else {
    console.log(`SELLING with wallet: ${wallet.classicAddress}`)
    appl.set(parseFloat(volume))
    const takerGets: IssuedCurrencyAmount = {
      value: String(appl.value),
      currency: appl.currency as string,
      issuer: appl.issuer as string,
    }
    const takerPays: string = xrpToDrops(
      String(parseInt(String(parseFloat(price) * (appl.value as number))))
    )
    await sell(testContext, wallet, takerGets, takerPays)
  }

  await testContext.client.disconnect()

  return
}

init()
setInterval(runCodeEvery10Seconds, 15000)
