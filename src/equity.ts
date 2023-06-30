import {
  Wallet,
  Payment,
  TrustSet,
  SetHookFlags,
  TransactionMetadata,
  xrpToDrops,
} from '@transia/xrpl'
import { IssuedCurrencyAmount } from '@transia/xrpl/dist/npm/models/common'
import { AccountID, Currency } from '@transia/ripple-binary-codec/dist/types'

import {
  createHookPayload,
  setHooksV3,
  SetHookParams,
  Xrpld,
  ExecutionUtility,
} from '@transia/hooks-toolkit'

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

const min = 200
const max = 300
const randomValue = (Math.random() * (max - min) + min).toFixed(2)

async function runCodeEvery10Seconds() {
  const testContext = (await setupClient(
    serverUrl
  )) as XrplIntegrationTestContext

  const aiWallet = Wallet.fromSeed('ssbyL7YhF7AURcLR2pFaC9CRvrp31')
  const applWallet = Wallet.fromSeed('shpxfmH1W3Fqt7kvsx3Dqni5neGfD')
  console.log(applWallet)

  // FUND AI WALLET
  if ((await balance(testContext.client, aiWallet.classicAddress)) < 2000) {
    const payTx: Payment = {
      TransactionType: 'Payment',
      Account: testContext.master.classicAddress,
      Destination: aiWallet.classicAddress,
      Amount: xrpToDrops(2000),
    }
    await Xrpld.submit(testContext.client, {
      wallet: testContext.master,
      tx: payTx,
    })
  }

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

  let previousValue = parseFloat(randomValue)

  function walkValue() {
    const minChange = -0.5
    const maxChange = 0.5
    const change = (
      Math.random() * (maxChange - minChange) +
      minChange
    ).toFixed(2)
    const newValue = (previousValue + parseFloat(change)).toFixed(2)
    previousValue = parseFloat(newValue)
    return newValue
  }

  console.log(`NEW VALUE: ${walkValue()}`)
  const value = walkValue()
  const appl = IC.gw('APL', applWallet.classicAddress)
  const limitAmount1: IssuedCurrencyAmount = {
    value: String(value),
    currency: appl.currency as string,
    issuer: appl.issuer as string,
  }
  const trustTx2: TrustSet = {
    TransactionType: 'TrustSet',
    Account: aiWallet.classicAddress,
    LimitAmount: limitAmount1,
  }
  console.log(trustTx2.LimitAmount)

  await Xrpld.submit(testContext.client, {
    wallet: aiWallet,
    tx: trustTx2,
  })
  await testContext.client.disconnect()
}

setInterval(runCodeEvery10Seconds, 15000)
