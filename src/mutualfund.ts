import {
  Wallet,
  Payment,
  TrustSet,
  SetHookFlags,
  TransactionMetadata,
  xrpToDrops,
  AccountSet,
  AccountSetAsfFlags,
  convertStringToHex,
} from '@transia/xrpl'
import { IssuedCurrencyAmount } from '@transia/xrpl/dist/npm/models/common'
import { AccountID, Currency } from '@transia/ripple-binary-codec/dist/types'

import {
  createHookPayload,
  setHooksV3,
  SetHookParams,
  Xrpld,
  ExecutionUtility,
  iHookParamName,
  iHookParamValue,
  iHookParamEntry,
  StateUtility,
} from '@transia/hooks-toolkit'

// NOT EXPORTED
import {
  IC,
  XrplIntegrationTestContext,
  accountSet,
  balance,
  close,
  limit,
  serverUrl,
  setupClient,
  trust,
} from '@transia/hooks-toolkit/dist/npm/src/libs/xrpl-helpers'

export async function setHookAccount(): Promise<void> {
  const testContext = (await setupClient(
    serverUrl
  )) as XrplIntegrationTestContext

  const hookWallet = Wallet.fromSeed('sEd7b3Tpn4aMuHrdWeyhmAQJKXYBf5j')
  const signerWallet = Wallet.fromSeed('sne9DmyPVfwWLKQyADZqVeByonn5v')
  const publicKey = signerWallet.publicKey

  // AccountSet
  const accountSetTx: AccountSet = {
    TransactionType: 'AccountSet',
    Account: hookWallet.classicAddress,
    TransferRate: 0,
    Domain: convertStringToHex('https://usd.transia.io'),
    SetFlag: AccountSetAsfFlags.asfGlobalFreeze,
  }
  await Xrpld.submit(testContext.client, {
    wallet: hookWallet,
    tx: accountSetTx,
  })

  if ((await balance(testContext.client, signerWallet.classicAddress)) < 2000) {
    const payTx: Payment = {
      TransactionType: 'Payment',
      Account: testContext.master.classicAddress,
      Destination: signerWallet.classicAddress,
      Amount: xrpToDrops(2000),
    }
    await Xrpld.submit(testContext.client, {
      wallet: testContext.master,
      tx: payTx,
    })
  }

  if ((await balance(testContext.client, hookWallet.classicAddress)) < 2000) {
    const payTx: Payment = {
      TransactionType: 'Payment',
      Account: testContext.master.classicAddress,
      Destination: hookWallet.classicAddress,
      Amount: xrpToDrops(2000),
    }
    await Xrpld.submit(testContext.client, {
      wallet: testContext.master,
      tx: payTx,
    })

    const limitAmount: IssuedCurrencyAmount = {
      value: '10000',
      currency: testContext.ic.currency as string,
      issuer: testContext.ic.issuer as string,
    }
    const trustTx: TrustSet = {
      TransactionType: 'TrustSet',
      Account: hookWallet.classicAddress,
      LimitAmount: limitAmount,
    }
    await Xrpld.submit(testContext.client, {
      wallet: hookWallet,
      tx: trustTx,
    })
  }

  // Hook #1 NAV
  const param1 = new iHookParamEntry(
    new iHookParamName('PK'),
    new iHookParamValue(publicKey, true)
  )
  const hook1 = createHookPayload(
    0,
    'nav',
    'mutualfund',
    SetHookFlags.hsfOverride,
    ['Invoke'],
    [param1.toXrpl()]
  )

  // Hook #2 FUND
  const hook2 = createHookPayload(
    0,
    'mutualfund',
    'mutualfund',
    SetHookFlags.hsfOverride,
    ['Payment']
  )

  await setHooksV3({
    client: testContext.client,
    seed: hookWallet.seed,
    hooks: [{ Hook: hook1 }, { Hook: hook2 }],
  } as SetHookParams)

  await testContext.client.disconnect()
}

export async function buyNAVAlice(): Promise<void> {
  const testContext = (await setupClient(
    serverUrl
  )) as XrplIntegrationTestContext

  const hookWallet = Wallet.fromSeed('sEd7b3Tpn4aMuHrdWeyhmAQJKXYBf5j')
  const aliceWallet = testContext.alice
  const nav = IC.gw('NAV', hookWallet.classicAddress)
  const limitValue = 100000
  if (
    (await limit(testContext.client, aliceWallet.classicAddress, nav)) <
    limitValue
  ) {
    console.log('ADD NAV TRUSTLINE')
    const limitAmount1: IssuedCurrencyAmount = {
      value: String(limitValue),
      currency: nav.currency as string,
      issuer: nav.issuer as string,
    }
    const trustTx2: TrustSet = {
      TransactionType: 'TrustSet',
      Account: aliceWallet.classicAddress,
      LimitAmount: limitAmount1,
    }
    await Xrpld.submit(testContext.client, {
      wallet: aliceWallet,
      tx: trustTx2,
    })
  }

  // PAYMENT IN

  const amount: IssuedCurrencyAmount = {
    value: '100',
    currency: testContext.ic.currency as string,
    issuer: testContext.ic.issuer as string,
  }

  const builtTx: Payment = {
    TransactionType: 'Payment',
    Account: aliceWallet.classicAddress,
    Destination: hookWallet.classicAddress,
    Amount: amount,
  }

  const result = await Xrpld.submit(testContext.client, {
    wallet: aliceWallet,
    tx: builtTx,
  })

  await close(testContext.client)

  const hookExecutions = await ExecutionUtility.getHookExecutionsFromMeta(
    testContext.client,
    result.meta as TransactionMetadata
  )
  console.log(hookExecutions.executions[0].HookReturnString)
  await testContext.client.disconnect()
}

export async function report(): Promise<void> {
  const testContext = (await setupClient(
    serverUrl
  )) as XrplIntegrationTestContext
  const hookWallet = Wallet.fromSeed('sEd7b3Tpn4aMuHrdWeyhmAQJKXYBf5j')

  const hookState1 = await StateUtility.getHookStateDir(
    testContext.client,
    hookWallet.classicAddress,
    'mutualfund'
  )
  console.log(hookState1)

  // console.log(`ALICE NAV: ${aliceNAVBal}`)
  await testContext.client.disconnect()
}

setHookAccount().then(() => {
  report()
  buyNAVAlice()
})
