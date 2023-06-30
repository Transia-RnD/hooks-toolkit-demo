import {
  Wallet,
  Payment,
  Invoke,
  SetHookFlags,
  TransactionMetadata,
  xrpToDrops,
} from '@transia/xrpl'
// import { IssuedCurrencyAmount } from '@transia/xrpl/dist/npm/models/common'
import { Currency } from '@transia/ripple-binary-codec/dist/types'

import {
  createHookPayload,
  setHooksV3,
  SetHookParams,
  Xrpld,
  ExecutionUtility,
  iHookParamEntry,
  iHookParamName,
  iHookParamValue,
} from '@transia/hooks-toolkit'

// NOT EXPORTED
import {
  // IC,
  XrplIntegrationTestContext,
  balance,
  close,
  serverUrl,
  setupClient,
  // trust,
} from '@transia/hooks-toolkit/dist/npm/src/libs/xrpl-helpers'

import { ProposalModel } from './models/ProposalModel'
import { decodeModel } from '@transia/hooks-toolkit/dist/npm/src/libs/binary-models'

const applWallet = Wallet.fromSeed('shpxfmH1W3Fqt7kvsx3Dqni5neGfD')

export async function setHook(): Promise<void> {
  const testContext = (await setupClient(
    serverUrl
  )) as XrplIntegrationTestContext

  const hookWallet = Wallet.fromSeed('sEd7QevWKtfC4nk2C6NxqyUU2pTViSc')
  // console.log(hookWallet);

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
  }

  const hook = createHookPayload(
    0,
    'dao_proposal',
    'dao_proposal',
    SetHookFlags.hsfOverride,
    ['Invoke']
  )

  await setHooksV3({
    client: testContext.client,
    seed: hookWallet.seed,
    hooks: [{ Hook: hook }],
  } as SetHookParams)

  await testContext.client.disconnect()
}

export async function createProposalAlice(): Promise<void> {
  const testContext = (await setupClient(
    serverUrl
  )) as XrplIntegrationTestContext

  const hookWallet = Wallet.fromSeed('sEd7QevWKtfC4nk2C6NxqyUU2pTViSc')
  // console.log(hookWallet);

  const proposalModel = new ProposalModel(
    BigInt('1691012112'), // expiration
    0, // required signatures
    0, // 0 buy 1 sell
    100, // value
    'APL', // currency
    applWallet.classicAddress // address
  )

  console.log(proposalModel)

  const param1 = new iHookParamEntry(
    new iHookParamName('P'),
    new iHookParamValue(proposalModel.encode(), true)
  )

  // INVOKE IN
  const aliceWallet = testContext.alice
  const builtTx: Invoke = {
    TransactionType: 'Invoke',
    Account: aliceWallet.classicAddress,
    Destination: hookWallet.classicAddress,
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
  const hookWallet = Wallet.fromSeed('sEd7QevWKtfC4nk2C6NxqyUU2pTViSc')

  await testContext.client.disconnect()
}

// report()
setHook().then(() => {
  createProposalAlice()
})

// const applWallet = Wallet.fromSeed('shpxfmH1W3Fqt7kvsx3Dqni5neGfD')
// // console.log(applWallet)

// const proposal = new ProposalModel(
//   BigInt('1688333712'),
//   3,
//   10,
//   'APL',
//   'r43yF68Qw1dWNcxtoHhWEhoFwF4y5nmmkc'
// )

// console.log(Currency.from('APL').toHex())
// // console.log(AccountID.from(applWallet.classicAddress).toHex())

// console.log(proposal)
// console.log(proposal.encode())

// console.log(
//   decodeModel(
//     '0000000064A1ED90030080C6A47E8DC35441504C0000000000000000000000000000000000E7DCFC77928AC15CABE9C5C2C9CA57B51FFD6110',
//     ProposalModel
//   )
// )
